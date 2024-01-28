#include "DrawSystemNcurses.hpp"

const char* TheWarning = R"EOL(
~~~~~~~~~~~~~~~~WARNING!~~~~~~~~~~~~~~~~~~~
This program produces flashing images which
   may trigger individuals with epilepsy.  

To continue, press 'y'.  
Press any other key to exit.  
)EOL";

int main(int, char**) {
	{ //TODO: NCurses shouldn't be a specific requirement;
	NCursesDrawer NCD;
	ncurses_WindowHandle Win(11,48,NCD.GetWindowSize().Y/2-5,NCD.GetWindowSize().X/2-(48/2),' ');
	mvwprintw(Win.GetHandle(),0,0,"%s",TheWarning);
	Win.Refresh();
	NCD.Refresh();
	timeout(-1);
	getch();
	}

	MainWindow<NCursesDrawer,ncurses_InputPipe> Win;
	bool Running = true;
	while (Running) {
		Win.HandleInput(Running);
		Win.Draw();
		Win.Refresh();
	}
}
