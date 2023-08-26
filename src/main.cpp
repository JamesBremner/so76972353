#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <wex.h>
#include <GraphTheory.h>

#include "cSolver.h"
#include "cGUI.h"

/// @brief true if name is in strings
/// @param strings
/// @param name
bool is_in(
    const std::vector<std::string> &strings,
    const std::string &name)
{
    return (std::find(
                strings.begin(), strings.end(),
                name) != strings.end());
}

void cSolver::sanity()
{
    // check match set includes root node type
    fRootInMatch = is_in(match, root.substr(1, 1));
    candidates.clear();
    subGraphs.clear();
}
void cSolver::removeCandidatesNotInMatch()
{
    candidates.erase(
        std::remove_if(
            candidates.begin(), candidates.end(),
            [this](const std::string &c) -> bool
            {
                return (!is_in(match, c.substr(1, 1)));
            }),
        candidates.end());
}

bool cSolver::isPathToRoot(int vi)
{
    // all paths from v to root
    gd.startName = root;
    gd.endName = gd.g.userName(vi);
    auto vp = dfs_allpaths(gd);
    if (!vp.size())
    {
        // v not reachable from root
        return false;
    }
    for (auto &p : vp)
    {
        bool pathOK = true;
        for (int u : p)
        {
            if (!is_in(
                    match,
                    gd.g.userName(u).substr(1, 1)))
            {
                // v not reachable through atom types in match
                pathOK = false;
                break;
            }
        }
        if (pathOK)
            return true;
    }
    return false;
}

void cSolver::addCandidatesReachableFromRoot()
{
    for (int v = 0; v < gd.g.vertexCount(); v++)
    {
        auto name = gd.g.userName(v);

        // check for already a candidate
        if (is_in(candidates, name))
            continue;

        // check for type in match list
        if (!is_in(match, name.substr(1, 1)))
            continue;

        // check root reachable via vertices with matching types
        if (!isPathToRoot(v))
            continue;

        candidates.push_back(name);
    }
}

void cSolver::sortCandidates()
{
    std::sort(candidates.begin(), candidates.end(),
              [](const std::string &a, const std::string &b) -> bool
              {
                  return a < b;
              });
}
std::vector<std::string>
cSolver::listRootConnected()
{
    return gd.g.userName(gd.g.adjacentOut(gd.g.find(root)));
}
void cSolver::removeUnconnected()
{
    auto rootConnected = listRootConnected();
    std::vector<int> vrem;
    int k = -1;
    for (auto sg : subGraphs)
    {
        k++;
        for (auto v : sg)
        {
            if (std::find(rootConnected.begin(), rootConnected.end(), v) != rootConnected.end())
                continue;
            gd.startName = root;
            gd.endName = v;
            if (bfsPath(gd).size())
                continue;
            vrem.push_back(k);
        }
        for (int r : vrem)
            subGraphs.erase(
                subGraphs.begin() + r);
    }
}

void cSolver::addIfNovel(
    const std::vector<std::string> &vSelected)
{
    if (std::find_if(subGraphs.begin(), subGraphs.end(),
                     [&](const std::vector<std::string> &v) -> bool
                     {
                         return std::is_permutation(v.begin(), v.end(), vSelected.begin());
                     }) == subGraphs.end())
        subGraphs.push_back(vSelected);
}

std::vector<std::string>
cSolver::select()
{
    // a new empty selection
    std::vector<std::string> vSelected;

    // loop over candidates in permuted list
    for (auto &consider : candidates)
    {
        std::string considerType = consider.substr(1, 1);

        // count number of this type required
        int cm = std::count(match.begin(), match.end(), considerType);

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
    if (!vSelected.size())
        return vSelected;

    // add root if not in match set
    if (!fRootInMatch)
        vSelected.insert(vSelected.begin(), root);

    addIfNovel(vSelected);

    return vSelected;
}

void cSolver::generate1()
{
    gd.g.clear();
    gd.g.add("2B", "1A");
    gd.g.add("2B", "3A");
    gd.g.add("2B", "4A");
    gd.g.add("2B", "5C");
    gd.g.add("3A", "10D");

    std::vector<std::string> m{
        "B", "A", "A"};
    match = m;

    root = "2B";
}

void cSolver::setMatch(
    const std::string &s)
{
    match.clear();
    std::stringstream sst(s);
    std::string a;
    while (getline(sst, a, ' '))
        match.push_back(a);
}
bool cSolver::setRoot(
    const std::string &s)
{
    for (int v = 0; v < gd.g.vertexCount(); v++)
    {
        auto name = gd.g.userName(v);
        if (name.substr(0, 1) == s)
        {
            root = name;
            return true;
        }
    }
    return false;
}

bool cSolver::read(
    const std::string &fname)
{
    gd.g.clear();
    std::ifstream ifs(fname);
    if (!ifs.is_open())
        return false;
    std::string va, vb;
    ifs >> va >> vb;
    while (ifs.good())
    {
        gd.g.add(va, vb);
        ifs >> va >> vb;
    }
    return true;
}

void cSolver::solve()
{
    sanity();

    // atoms connected directly to root
    candidates = listRootConnected();
    removeCandidatesNotInMatch();

    addCandidatesReachableFromRoot();

    sortCandidates();

    // loop over permuted lists of candidates
    do
    {
        select();

    } while (std::next_permutation(
        candidates.begin(), candidates.end()));

    // remove subgraphs that are not connected
    removeUnconnected();
}

std::vector<std::string>
cSolver::text()
{
    std::vector<std::string> vs;
    vs.push_back("Results");
    for (auto &v : subGraphs)
    {
        std::string line;
        for (auto s : v)
            line += s + " ";
        vs.push_back(line);
    }
    return vs;
}

cGUI::cGUI()
    : cStarterGUI(
          "so76972353",
          {50, 50, 1000, 500}),
      mylbRoot(wex::maker::make<wex::label>(fm)),
      mylbMatch(wex::maker::make<wex::label>(fm)),
      myEdit(wex::maker::make<wex::editbox>(fm)),
      myedRoot(wex::maker::make<wex::editbox>(fm)),
      mybnRead(wex::maker::make<wex::button>(fm)),
      mybn(wex::maker::make<wex::button>(fm))
{
    // generate problem instance from specifications
    solver.generate1();

    int y = 20;
    mybnRead.move(20, y, 100, 30);
    mybnRead.text("Read Molecule");
    y += 35;
    mylbRoot.move(20, y, 70, 25);
    mylbRoot.text("Root Index");
    myedRoot.move(90, y, 50, 25);
    myedRoot.text("");
    y += 30;
    mylbMatch.move(20, y, 50, 25);
    mylbMatch.text("Set");
    myEdit.move(90, y, 200, 25);
    myEdit.text("");
    y += 30;
    mybn.move(20, y, 100, 30);
    mybn.text("SOLVE");

    mybnRead.events().click(
        [&]
        {
            wex::filebox fb(fm);
            if (!solver.read(fb.open()))
            {
                wex::msgbox("Error\nCannot read file");
                return;
            }
        });

    mybn.events().click(
        [&]
        {
            if (!solver.setRoot(myedRoot.text()))
            {
                wex::msgbox("Error\nCannot find root");
                return;
            }
            solver.setMatch(myEdit.text());
            solver.solve();
            fm.update();
        });

    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            int row = 0;
            for (auto &line : solver.text())
            {
                S.text(line, {80, 150 + 50 * row++, 200, 25});
            }
        });

    show();
    run();
}

main()
{
    cGUI theGUI;
    return 0;
}
