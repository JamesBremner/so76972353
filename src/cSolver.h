class cSolver
{
    raven::graph::sGraphData gd; // graph representation of molecule
    std::string root;
    std::vector<std::string> match; // required atoms
    std::vector<std::string> candidates;
    std::vector<std::vector<std::string>> subGraphs;
    bool fRootInMatch;

    void sanity();
    void removeCandidatesNotInMatch();

    /// @brief Is there path from vertex to root through vertex types in match set
    /// @param vi
    /// @return
    bool isPathToRoot(int vi);

    void addCandidatesReachableFromRoot();

    void sortCandidates();

    std::vector<std::string>
    listRootConnected();

    void removeUnconnected();

    void addIfNovel(
        const std::vector<std::string> &vSelected);

    /// @brief select subgraph using first available atoms in permuted candidate list
    /// @return
    std::vector<std::string>
    select();

public:

    void generate1();
    void setMatch(
        const std::string &s);
    bool setRoot(
        const std::string &s);
    bool read(
        const std::string &fname );

    void solve();

    std::vector<std::string>
    text();


};