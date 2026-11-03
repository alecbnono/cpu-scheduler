#include <iostream>
#include <string>

#include "controller/Controller.hpp"
#include "model/Model.hpp"
#include "view/View.hpp"

int main() {
    Model model;
    View view;
    Controller controller(model, view);

    view.showMessage("Welcome to CSOPESY!.\n");
    view.showMessage("Group Developers:");
    view.showMessage("Nono, Alec Marx");
    view.showMessage("Obregon, Sian Ysabelle");
    view.showMessage("Ponce, Jean Rondel");
    view.showMessage("Sy, Prince Matthew\n");
    view.showMessage("Type 'help' for a list of commands.");

    std::string line;
    bool running = true;
    while (running) {
        view.showPrompt();
        if (!std::getline(std::cin, line)) {
            break;
        }
        running = controller.handleCommand(line);
    }

    return 0;
}
