#pragma once

#include "Subsystem.hpp"

#include <boost/program_options.hpp>

#include <vector>

namespace po = boost::program_options;

namespace jar {

class Application : public Subsystem {
public:
    static Application&
    instance();

    ~Application() override;

    void
    parseArgs(int argc, char* argv[]);

    int
    run();

    [[nodiscard]] const po::variables_map&
    options() const;

protected:
    Application();

    virtual void
    defineOptions(po::options_description& description);

    bool
    waitForTermination();

    void
    addSubsystem(Subsystem::Ptr subsystem);

    void
    initialize(Application& application) override;

    void
    setUp(Application& application) override;

    void
    tearDown() override;

    void
    finalize() override;

    virtual void
    proceed()
        = 0;

private:
    void
    processOptions(int argc, char* argv[]);

    void
    handleHelp(const po::options_description& description);

private:
    using Subsystems = std::vector<Subsystem::Ptr>;

    Subsystems _subsystems;
    po::variables_map _options;
    bool _helpRequested;

    static Application* s_instance;
};

} // namespace jar