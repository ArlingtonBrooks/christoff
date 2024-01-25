#ifndef DRAW_NCURSES_HPP_
#define DRAW_NCURSES_HPP_

/** @file Ncurses Drawing Functions
 * @brief Contains interface implementations for drawing an Ncurses interface
 */

#include "Interface.hpp"
#include "Types.hpp"

#include <ncurses.h>
#include <string> //string
#include <memory> //unique_ptr
#include <unordered_map> //unordered_map

/** @brief Input handler for ncurses */
struct ncurses_InputHandler : public InputHandler {
private:
	WINDOW* Handle;    ///<NON-OWNING POINTER
	int timeout;       ///<Stored timeout

	/** @brief Process a character from input 
	 * @param Input       Input to be processed
	 * @param ToModify    The string to which the input is being added
 	 */
	static bool ProcessInput(int Input, std::string &ToModify) {
		bool IsValidChar = false;
		IsValidChar |= (Input >= 32 && Input <= 126);
		if (IsValidChar) {
			ToModify.push_back((char)Input);
			return true;
		}
		else {return false;}
	}
public:
	/** 
	 * @param tHandle      Handle to the ncurses window
	 * @param ttimeout     Nominal timeout of this window
	 */
	ncurses_InputHandler(WINDOW* tHandle, int ttimeout = 0) {
		Handle = tHandle;
		timeout = ttimeout;
	}

	virtual int GetCharInput() override {
		return ::wgetch(Handle);
	}

	/** @todo: this would be nice if it could make a textbox popup */
	virtual std::string GetStringInput() override {
		::wtimeout(Handle,0);
		std::string str;
		int Input = ::wgetch(Handle);
		ProcessInput(Input,str);
		while (Input != '\n' && Input != KEY_ENTER) {
			Input = ::wgetch(Handle);
			ProcessInput(Input,str);
		}
		::wtimeout(Handle,timeout);
		return str;
	}
};

/** @brief An ncurses window object */
struct ncurses_WindowHandle : public WindowHandle {
private:
	WINDOW* Handle;           ///<Owning pointer
	int m_timeout = 0;        ///<Stored timeout
	bool m_Active = false;    ///<Whether this is an active window
public:
	ncurses_WindowHandle(const ncurses_WindowHandle&) = delete;
	ncurses_WindowHandle& operator=(const ncurses_WindowHandle&) = delete;
	ncurses_WindowHandle(ncurses_WindowHandle&&) = delete;
	ncurses_WindowHandle& operator=(ncurses_WindowHandle&&) = delete;

	ncurses_WindowHandle(WINDOW* tHandle) { //This takes ownership
		Handle = tHandle;
		m_Active = true;
	}
	ncurses_WindowHandle(int height, int width, int starty, int startx, char border = 0) {
		Handle = newwin(height,width,starty,startx);
		box(Handle,0,0);
		if (border != 0) {
			wborder(Handle, border, border, border, border, border, border, border, border);
		}
		m_Active = true;
		wrefresh(Handle);
	}
	virtual ~ncurses_WindowHandle() {
		wclear(Handle);
		werase(Handle);
		delwin(Handle);
	}

	virtual bool IsActive() const override {return m_Active;}

	/** @brief Return a handle to the window */
	WINDOW* GetHandle() {return Handle;}

	/** @brief Refresh the window */
	virtual void refresh() override {
		::wrefresh(Handle);
	}

	/** @brief Redraw whole window */
	virtual void redraw() override {
		::touchwin(Handle);
	}

	/** @brief Draw a character 
	 * @param Y     Y-location to draw the character
	 * @param X     X-location to draw the character
	 * @param bg_col    Background colour to be drawn
	 * @param N_to_print   Number of characters to repeat
	 * @param attributes   Text attributes
	 */
	void printchar(int Y, int X, int ToPrint, short bg_col = 0, int N_to_print = 1, attr_t attributes = A_NORMAL) {
		for (int i = 0; i != N_to_print; i++) {
			mvwprintw(Handle,Y,X,"%c",ToPrint);
		}
		mvwchgat(Handle,Y,X,N_to_print,attributes,bg_col,NULL);
	}

	/** @brief Number of milliseconds to block input (negative for infinite) */
	void SetInputTimeout(int time) {
		m_timeout = time;
		::wtimeout(Handle,time);
	}

	/** @brief Get the size of the current window */
	virtual BoxSize<int> GetSize() override {
		BoxSize<int> ret;
		getmaxyx(Handle,ret.Y,ret.X);
		return ret;
	}

	/** @brief Resize the window */
	void resize(int Ysz, int Xsz) {
		::wresize(Handle,Ysz,Xsz);
	}

	/** @brief Move the window */
	void move(int Yloc, int Xloc) {
		::mvwin(Handle,Yloc,Xloc);
	}
};

struct NCursesDrawer : public Drawer {
private:
	std::size_t UI_Hash = 0;
	std::unique_ptr<ncurses_InputHandler> m_Input;
	std::unordered_map<std::string,std::unique_ptr<ncurses_WindowHandle>> m_Children;
public:
	NCursesDrawer() {
		initscr();
		noraw();
		echo();
		cbreak();
		curs_set(0);
		noqiflush();
		keypad(stdscr,true);
		timeout(64);
		refresh();
		m_Input = std::make_unique<ncurses_InputHandler>(ncurses_InputHandler(stdscr));
	}

	~NCursesDrawer() {
		m_Children.clear();
		endwin();
	}

	/** Redraw everything on screen */
	virtual void Redraw() override {
		for (auto &Window : m_Children) {
			Window.second->redraw();
			Window.second->refresh();
		}
		Refresh();
	}

	/** Refresh all windows */
	virtual void Refresh() override {
		for (auto &Window : m_Children) {
			Window.second->refresh();
		}
		::refresh();
	}

	/** Get window size */
	virtual BoxSize<int> GetWindowSize() override {
		BoxSize<int> ret;
		getmaxyx(stdscr,ret.Y,ret.X);
		return ret;
	}

	/** Print the user interface in its current state */
	virtual void PrintUI(UserInterface &UI) override {
		std::size_t NewUI = UI.hash();
		if (NewUI == UI_Hash) {return;}
		else {UI_Hash = NewUI;}
		WINDOW* tUI = m_Children["InputWindow"]->GetHandle();
		//wclear(tUI);
		//box(tUI,0,0);
		int nLabels = UI.NumberOfLabels();
		for (int i = 0; i != nLabels; i++) {
			if (i == UI.CurrentSelection) {wattrset(tUI,A_STANDOUT);}
			mvwprintw(tUI,i+1,1,"%s",UI.GetLabel(i).c_str());
			wattrset(tUI,A_NORMAL);
			wprintw(tUI,"      ");
		}
	}
	
	virtual void UpdateVisual(UserInterface const &UI) override {};

	/** Create window for handling user inputs */
	virtual void CreateInputWindow() override {
		BoxSize<int> WinSize = GetWindowSize();
		switch (Orientation) {
		case Location::North: { //create box on top 7 pixels
			m_Children.emplace("InputWindow",std::make_unique<ncurses_WindowHandle>(7,WinSize.X,0,0));
			break;
		}
		//case Location::South: //create box on bottom 7 pixels
		//case Location::East:  //create east
		//case Location::West: //create west
		default: return;
		}
	}

	/** Create window for output of visuals */
	virtual void CreateVisualWindow() override { //TODO: check if window is oriented the same as the window manager;
		BoxSize<int> WinSize = GetWindowSize();
		switch (Orientation) {
		case Location::North: { //create box on top 7 pixels
			m_Children.emplace("VisualWindow",std::make_unique<ncurses_WindowHandle>(WinSize.Y-7,WinSize.X,7,0));
			break;
		}
		//case Location::South: //create box on bottom 7 pixels
		//case Location::East:  //create east
		//case Location::West: //create west
		default: return;
		}
	}
};

struct ncurses_InputPipe : public PipeInputToUI {
private:
	void HandleSelectionKey(UserInterface &UI) {
		switch ((UserInterface::Selection)(UI.CurrentSelection)) {
		case UserInterface::Selection::TIMESIGNATURE: break; //need to create window
		case UserInterface::Selection::BEATSPERMIN: break; //need to create window
		case UserInterface::Selection::COLORSEL: break; //Not applicable
		case UserInterface::Selection::VISUALIZATION: break; //Not applicable
		case UserInterface::Selection::FLASHING: UI.ToggleFlashing(); break;
		default: break;
		}
	}
	void HandleArrowKey(UserInterface &UI, char Direction) {
		switch ((UserInterface::Selection)(UI.CurrentSelection)) {
		case UserInterface::Selection::TIMESIGNATURE: break; //N/A
		case UserInterface::Selection::BEATSPERMIN: 
			UI.BPM += (Direction > 0) - (UI.BPM > 1.0f && Direction < 0); 
			break;
		case UserInterface::Selection::COLORSEL:
			UI.SetColor((Direction > 0) - (Direction < 0));
			break;
		case UserInterface::Selection::VISUALIZATION:
			UI.SetVisualization(Direction);
			break;
		case UserInterface::Selection::FLASHING: 
			UI.ToggleFlashing(); 
			break;
		default: break;
		}
	}
public:
	virtual int Keyboard(UserInterface &UI) override {
		int Input = getch();
		if (Input == ERR) return Input;
		if (Input == KEY_UP) {UI.MoveSelection(-1);}
		else if (Input == KEY_DOWN) {UI.MoveSelection(1);}
		else if (Input == '\n' || Input == KEY_ENTER) {HandleSelectionKey(UI);}
		else if (Input == KEY_LEFT) {HandleArrowKey(UI,-1);}
		else if (Input == KEY_RIGHT) {HandleArrowKey(UI,1);}
		return Input;
	}
};

#endif
