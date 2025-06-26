#include <QApplication>
#include "game_view.h"
#include "game_controller.h"
#include "game_model.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    game_model model;
    game_view view;
    game_controller controller(&model);

    view.setModel(&model);
    view.connect(&view, &game_view::connectRequested, &controller, &game_controller::connectToServer);
    view.connect(&view, &game_view::cellClicked, &controller, &game_controller::processCellClick);

    view.show();
    return app.exec();
}