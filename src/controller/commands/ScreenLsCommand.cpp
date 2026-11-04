#include "controller/commands/ScreenLsCommand.h"
#include "view/ProcessReportView.hpp"

ScreenLsCommand::ScreenLsCommand(const ISnapshotProvider& provider, std::ostream& out)
    : provider_(provider), out_(out) {}

bool ScreenLsCommand::execute(const std::vector<std::string>& /*args*/) {
    SystemSnapshot snapshot = provider_.getSnapshot();
    ProcessReportView::renderReport(out_, snapshot);
    return true;
}

ScreenLsCommand::ScreenLsCommand(const ISnapshotProvider& provider, View& /*view*/)
    : ScreenLsCommand(provider, std::cout) {}
