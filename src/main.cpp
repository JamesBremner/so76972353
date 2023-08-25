#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "cStarterGUI.h"

std::vector<std::vector<std::string>>
listMatchedAtoms(
    const std::vector<std::string> &toMatch,
    const std::vector<std::string> &Candidates)
{
    std::vector<std::vector<std::string>> O;

    // Start with vector of candidates sorted into lexigraphic order
    auto vc = Candidates;
    std::sort(vc.begin(), vc.end(),
              [](const std::string &a, const std::string &b) -> bool
              {
                  return a < b;
              });

    // loop over permuted lists of candidate
    do
    {
        // a new empty selection
        std::vector<std::string> vSelected;

        // loop over candidate in permuted list
        for (auto &consider : vc)
        {
            std::string considerType = consider.substr(0,1);

            int cm = std::count(toMatch.begin(), toMatch.end(),considerType);

            // count number of of this type selected so far
            int cSelected = std::count_if(vSelected.begin(), vSelected.end(),
                                          [&](const std::string &b) -> bool
                                          {
                                              return considerType[0] == b[0];
                                          });
            
            // add to selection if another of this type is required
            if (cm > cSelected)
                vSelected.push_back(consider);
        }

        // check we have a selection
        if (vSelected.size())
        {
            // check that this a unique selection
            if (std::find_if(O.begin(), O.end(),
                          [&](const std::vector<std::string> & v) -> bool
                          {
                              return std::is_permutation(v.begin(), v.end(), vSelected.begin());
                          }) == O.end())
            {
                // add selection to output list of selection
                O.push_back(vSelected);
            }
        }

    } while (std::next_permutation(vc.begin(), vc.end()));

    std::cout << "\nO\n";
    for( auto& v : O ) {
        for( auto s : v )
            std::cout << s << " ";
        std::cout << "\n";
    }



    return O;
}

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "Starter",
              {50, 50, 1000, 500}),
          lb(wex::maker::make<wex::label>(fm))
    {
        lb.move(50, 50, 100, 30);
        lb.text("Hello World");

        show();
        run();
    }

private:
    wex::label &lb;
};

main()
{
    auto O = listMatchedAtoms(
        {"B", "A", "A"},
        {"B2", "A1", "A3", "A4"});

    cGUI theGUI;
    return 0;
}
