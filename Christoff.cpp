#include "DrawSystemNcurses.hpp"

int main(int, char**) {
	MainWindow<NCursesDrawer,ncurses_InputPipe> Win;
	bool Running = true;
	while (Running) {
		Win.HandleInput(Running);
		Win.Draw();
		Win.Refresh();
	}
}
