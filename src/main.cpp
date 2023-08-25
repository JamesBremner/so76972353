#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <GraphTheory.h>
#include "cStarterGUI.h"

struct sProblemInstance
{
    raven::graph::sGraphData gd; // graph representation of molecule
    std::string root;
    std::vector<std::string> match; // required atoms
    std::vector<std::string> candidates;
    std::vector<std::vector<std::string>> subGraphs;
} gPI;

void generate1()
{
    gPI.gd.g.clear();
    gPI.gd.g.add("2B", "1A");
    gPI.gd.g.add("2B", "3A");
    gPI.gd.g.add("2B", "4A");
    gPI.gd.g.add("2B", "5C");
    gPI.gd.g.add("3A", "10D");

    std::vector<std::string> m{
        "B", "A", "A"};
    gPI.match = m;

    gPI.root = "2B";
}

void findCandidates()
{
    // check set includes root node type
    if (std::find(gPI.match.begin(), gPI.match.end(), gPI.root.substr(1, 1)) == gPI.match.end())
        throw std::runtime_error(
            "match set does not include root type");

    raven::graph::cGraph &g = gPI.gd.g;

    // atoms connected directly to root
    gPI.candidates = g.userName(g.adjacentOut(g.find(gPI.root)));

    for (int v = 0; v < g.vertexCount(); v++)
    {
        auto name = g.userName(v);

        // check for alredy a candidate
        if (std::find(gPI.candidates.begin(), gPI.candidates.end(), name) != gPI.candidates.end())
            continue;

        // check for in match list
        if (std::find(gPI.match.begin(), gPI.match.end(), name.substr(1, 1)) == gPI.match.end())
            continue;

        // all paths from v to root
        gPI.gd.startName = gPI.root;
        gPI.gd.endName = g.userName(v);
        auto vp = dfs_allpaths(gPI.gd);
        if (!vp.size())
        {
            // v not reachable from root
            continue;
        }
        bool reachable = false;
        for (auto &p : vp)
        {
            bool pathOK = true;
            for (int u : p)
            {
                if (std::find(gPI.match.begin(), gPI.match.end(), g.userName(u).substr(1, 1)) == gPI.match.end())
                {
                    // v not reachable through atom types in match
                    pathOK = false;
                    break;
                }
            }
            if( pathOK )
            {
                reachable = true;
                break;
            }
        }

        if (!reachable)
            break;

        gPI.candidates.push_back(g.userName(v));
    }

    // remove if not in match list
    gPI.candidates.erase(
        std::remove_if(
            gPI.candidates.begin(), gPI.candidates.end(),
            [](const std::string &c) -> bool
            {
                return std::find(gPI.match.begin(), gPI.match.end(), c.substr(1, 1)) == gPI.match.end();
            }),
        gPI.candidates.end());

    std::cout << "Candidates: ";
    for (auto &c : gPI.candidates)
        std::cout << c << " ";
    std::cout << "\n";
}

void findSubGraphs()
{
    // Start with vector of candidates sorted into lexigraphic order
    auto vc = gPI.candidates;
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

        // loop over candidates in permuted list
        for (auto &consider : vc)
        {
            std::string considerType = consider.substr(1, 1);

            // count number of this type required
            int cm = std::count(gPI.match.begin(), gPI.match.end(), considerType);

            // count number of of this type selected so far
            int cSelected = std::count_if(vSelected.begin(), vSelected.end(),
                                          [&](const std::string &b) -> bool
                                          {
                                              return considerType[0] == b[1];
                                          });

            // add to selection if another of this type is required
            if (cm > cSelected)
                vSelected.push_back(consider);
        }

        // check we have a selection
        if (vSelected.size())
        {
            // check that this a unique selection
            if (std::find_if(gPI.subGraphs.begin(), gPI.subGraphs.end(),
                             [&](const std::vector<std::string> &v) -> bool
                             {
                                 return std::is_permutation(v.begin(), v.end(), vSelected.begin());
                             }) == gPI.subGraphs.end())
            {
                // add selection to output list of selection
                gPI.subGraphs.push_back(vSelected);
            }
        }

    } while (std::next_permutation(vc.begin(), vc.end()));

    // remove subgraphs that are not connected
    auto rootConnected = gPI.gd.g.userName(gPI.gd.g.adjacentOut(gPI.gd.g.find(gPI.root)));
    std::vector<int> vrem;
    int k = -1;
    for (auto sg : gPI.subGraphs)
    {
        k++;
        for (auto v : sg)
        {
            if (std::find(rootConnected.begin(), rootConnected.end(), v) != rootConnected.end())
                continue;
            gPI.gd.startName = gPI.root;
            gPI.gd.endName = v;
            if (bfsPath(gPI.gd).size())
                continue;
            vrem.push_back(k);
        }
        for (int r : vrem)
            gPI.subGraphs.erase(
                gPI.subGraphs.begin() + r);
    }

    std::cout << "\nResults\n";
    for (auto &v : gPI.subGraphs)
    {
        for (auto s : v)
            std::cout << s << " ";
        std::cout << "\n";
    }
}

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "so76972353",
              {50, 50, 1000, 500})
    {
        fm.events().draw(
            [&](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                display(S);
            });

        show();
        run();
    }

private:
    void display(wex::shapes &S)
    {
        int row = 0;
        for (auto &v : gPI.subGraphs)
        {
            std::string line;
            for (auto s : v)
                line += s + " ";
            S.text(line, {80, 50 + 50 * row++, 200, 25});
        }
    }
};

main()
{
    // generate problem instance from specifications
    generate1();

    // find candidate atoms that vould be included in fragments
    findCandidates();

    // list fragments
    findSubGraphs();

    cGUI theGUI;
    return 0;
}
