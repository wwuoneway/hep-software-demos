// Writes a std::vector<T> with TTree or RNTuple, reads it back, and reports what happened.
//
// usage: probe <point|pixel|ghost|secret> <check|ttree|rntuple>
//
//   point   std::vector<Point>   Point has a dictionary, std::vector<Point> has one too
//   pixel   std::vector<Pixel>   Pixel has a dictionary, std::vector<Pixel> does not
//   ghost   std::vector<Ghost>   no dictionary, but the interpreter can parse Ghost's header
//   secret  std::vector<Secret>  no dictionary, unknown to the interpreter (defined only here)
//
//   check    the two dictionary checks: top-level HasDictionary() and the RField walk
//   ttree    write with TTree, read back, compare
//   rntuple  write with RNTuple, read back, compare
//
// Each run is a separate process so a crash in one case cannot hide the others.

#include "Shapes.h"

#include <ROOT/RField.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>
#include <TClass.h>
#include <TInterpreter.h>
#include <TFile.h>
#include <TTree.h>

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using demo::Ghost;
using demo::Pixel;
using demo::Point;

// Known only to this program: no dictionary, and not in any header rootcling saw.
struct Secret {
  int value = 0;
  bool operator==(Secret const&) const = default;
};

namespace {

  template <class T>
    std::vector<T> sample();
  template <>
    std::vector<Point> sample() { return {{1.f, 2.f}, {3.f, 4.f}}; }
  template <>
    std::vector<Pixel> sample() { return {{1, 2}, {3, 4}}; }
  template <>
    std::vector<Ghost> sample() { return {{5, 1.5}, {6, 2.5}}; }
  template <>
    std::vector<Secret> sample() { return {{7}, {8}}; }

  std::string first_line(std::string const& text) { return text.substr(0, text.find('\n')); }

  // Reject when the top-level type's TClass has no dictionary.
  template <class T>
    std::string top_level_check()
    {
      auto* cl = TClass::GetClass(typeid(std::vector<T>));
      if (cl == nullptr) {
        return "no TClass";
      }
      return cl->HasDictionary() ? "accept" : "REJECT";
    }

  // Walks the RNTuple field tree: every class RNTuple stores member-wise (not an STL collection,
  // which RNTuple handles itself) must have a dictionary.
  bool classes_have_dictionaries(ROOT::RFieldBase const& field, std::string& missing)
  {
    auto* cl = TClass::GetClass(field.GetTypeName().c_str());
    if (cl != nullptr && cl->GetCollectionProxy() == nullptr && !cl->HasDictionary()) {
      missing = field.GetTypeName();
      return false;
    }
    for (auto const* sub : field.GetConstSubfields()) {
      if (!classes_have_dictionaries(*sub, missing)) {
        return false;
      }
    }
    return true;
  }

  std::string field_walk_check(std::string const& type_name)
  {
    try {
      auto field = ROOT::RFieldBase::Create("v", type_name).Unwrap();
      std::string missing;
      return classes_have_dictionaries(*field, missing) ? "accept" : "REJECT (" + missing + ")";
    } catch (std::exception const& e) {
      return "REJECT (RField::Create failed)";
    }
  }

  template <class T>
    int write_read_ttree(std::string const& file)
    {
      auto values = sample<T>();
      {
        TFile out(file.c_str(), "RECREATE");
        TTree tree("t", "t");
        auto* address = &values;
        if (tree.Branch("v", &address) == nullptr) {
          std::cout << "TTree: Branch() refused the type\n";
          return 2;
        }
        tree.Fill();
        tree.Write();
      }
      TFile in(file.c_str(), "READ");
      auto* tree = in.Get<TTree>("t");
      std::vector<T>* read_back = nullptr;
      tree->SetBranchAddress("v", &read_back);
      tree->GetEntry(0);
      bool const same = read_back != nullptr && *read_back == values;
      std::cout << "TTree: wrote " << values.size() << ", read "
        << (read_back ? read_back->size() : 0) << (same ? ", values match\n" : ", MISMATCH\n");
      return same ? 0 : 1;
    }

  template <class T>
    int write_read_rntuple(std::string const& file, std::string const& type_name)
    {
      auto values = sample<T>();
      try {
        auto model = ROOT::RNTupleModel::Create();
        model->AddField(ROOT::RFieldBase::Create("v", type_name).Unwrap());
        auto writer = ROOT::RNTupleWriter::Recreate(std::move(model), "n", file);
        auto entry = writer->CreateEntry();
        entry->BindRawPtr("v", &values);
        writer->Fill(*entry);
      } catch (std::exception const& e) {
        std::cout << "RNTuple: write failed: " << first_line(e.what()) << "\n";
        return 2;
      }
      auto reader = ROOT::RNTupleReader::Open("n", file);
      auto view = reader->GetView<void>("v");
      auto const read_back_ptr = view.GetField().CreateObject<std::vector<T>>();
      view.BindRawPtr(read_back_ptr.get());
      view(0);
      auto const& read_back = *read_back_ptr;
      bool const same = read_back == values;
      std::cout << "RNTuple: wrote " << values.size() << ", read " << read_back.size()
        << (same ? ", values match\n" : ", MISMATCH\n");
      return same ? 0 : 1;
    }

  template <class T>
    int run(std::string const& step, std::string const& type_name, std::string const& tag)
    {
      if (step == "check") {
        std::cout << "top-level HasDictionary(): " << top_level_check<T>() << "\n";
        std::cout << "RField walk:               " << field_walk_check(type_name) << "\n";
        return 0;
      }
      if (step == "ttree") {
        return write_read_ttree<T>("build/" + tag + "_ttree.root");
      }
      if (step == "rntuple") {
        return write_read_rntuple<T>("build/" + tag + "_rntuple.root", type_name);
      }
      std::cerr << "unknown step: " << step << "\n";
      return 64;
    }
}

int main(int argc, char** argv)
{
  if (argc != 3) {
    std::cerr << "usage: probe <point|pixel|ghost|secret> <check|ttree|rntuple>\n";
    return 64;
  }
  std::string const which{argv[1]};
  std::string const step{argv[2]};
  if (which == "point") {
    return run<Point>(step, "std::vector<demo::Point>", which);
  }
  if (which == "pixel") {
    return run<Pixel>(step, "std::vector<demo::Pixel>", which);
  }
  if (which == "ghost") {
    // Let the interpreter parse Shapes.h, as ROOT does when it meets a header on its include
    // path. Ghost then gets an interpreted TClass, but still no compiled dictionary.
    gInterpreter->Declare("#include \"Shapes.h\"");
    return run<Ghost>(step, "std::vector<demo::Ghost>", which);
  }
  if (which == "secret") {
    return run<Secret>(step, "std::vector<Secret>", which);
  }
  std::cerr << "unknown type: " << which << "\n";
  return 64;
}
