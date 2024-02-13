#ifndef DRAW_NCURSES_HPP_
#define DRAW_NCURSES_HPP_

/** @file Ncurses Drawing Functions
 * @brief Contains interface implementations for drawing an Ncurses interface
 */

#include "Interface.hpp"
#include "Types.hpp"
#include "Formulas.hpp"

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

	/** @todo: this would be nice if it could make a textbox or window popup */
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
	char m_border;
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
		m_border = border;
		m_Active = true;
		wrefresh(Handle);
	}
	virtual ~ncurses_WindowHandle() {
		wclear(Handle);
		werase(Handle);
		delwin(Handle);
	}

	/** @brief Return whether this is "active" or "alive" */
	virtual bool IsActive() const override {return m_Active;}

	/** @brief Return a handle to the window */
	WINDOW* GetHandle() {return Handle;}

	/** @brief Refresh the window */
	virtual void Refresh() override {
		::wrefresh(Handle);
	}

	/** @brief Redraw whole window */
	virtual void Redraw() override {
		wclear(Handle);
		box(Handle,0,0);
		if (m_border != 0) {
			wborder(Handle, m_border, m_border, m_border, m_border, m_border, m_border, m_border, m_border);
		}
		::touchwin(Handle);
	}

	/* Primitive draws */
	virtual void DrawCircle(float Radius, Position<float> const &Loc, ColorType<unsigned char> Border, float BorderThickness, bool Fill = false, ColorType<unsigned char> FillColor = {0,0,0,0}) override {
		Unused(Radius, Loc, Border, BorderThickness, Fill, FillColor);
		throw std::runtime_error("Not Implemented");
	}
	virtual void DrawTriangle(Position<float> const &Pt1, Position<float> const &Pt2, Position<float> const &Pt3, ColorType<unsigned char> Border, float BorderThickness, Position<float> const &Offset = {0,0}, bool Fill = false, ColorType<unsigned char> FillColor = {0,0,0,0}) override {
		Unused(Pt1, Pt2, Pt3, Border, BorderThickness, Offset, Fill, FillColor);
		throw std::runtime_error("Not Implemented");
	}
	virtual void DrawLine(Position<float> const &Pt1, Position<float> const &Pt2, float Thickness, Position<float> const &Offset = {0,0}) override {
		Unused(Pt1, Pt2, Thickness, Offset);
		throw std::runtime_error("Not Implemented");
	}

	/** Fill will be determined the following way:
	 * R G B channels will be used to determine colour pair, as colour pairs will be limited:
	 * R=2: BLUE
	 * R=3: TEAL
	 * R=4: GREEN
	 * R=5: ORANGE
	 * R=6: RED
	 * R=7: PURPLE
	 * R=8: BLACK
	 * R=9: WHITE
	 * A channel will determine whether a character is also printed to screen.  The A channel will print based on the following:
	 * A < 25:  print '`'
	 * A < 50:  print '"'
	 * A < 75:  print '-'
	 * A < 100: print '~'
	 * A < 125: print ':'
	 * A < 150: print '*'
	 * A < 175: print '+'
	 * A < 200: print '%'
	 * A < 225: print 'O'
	 * A < 250: print '8'
	 * A = 255: '#' and fills entire block;
	 */
	virtual void FillScreen(ColorType<unsigned char> FillColor) override {
		if (FillColor.A == 255) {
			if (FillColor.R > 10) FillColor.R -= 10;
			wbkgd(Handle,COLOR_PAIR(FillColor.R) | '#');
			return;
		} else {
			if (FillColor.R < 10) FillColor.R += 10;
		}
		if      (FillColor.A > 250) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '#');
		else if (FillColor.A > 225) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '8');
		else if (FillColor.A > 200) wbkgd(Handle,COLOR_PAIR(FillColor.R) | 'O');
		else if (FillColor.A > 175) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '%');
		else if (FillColor.A > 150) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '+');
		else if (FillColor.A > 125) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '*');
		else if (FillColor.A > 100) wbkgd(Handle,COLOR_PAIR(FillColor.R) | ':');
		else if (FillColor.A > 75 ) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '~');
		else if (FillColor.A > 50 ) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '-');
		else if (FillColor.A > 25 ) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '"');
		else if (FillColor.A > 0  ) wbkgd(Handle,COLOR_PAIR(FillColor.R) | '`');
		else wbkgd(Handle,' ');
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

/** @brief NCurses implementation of VisualOutput */
struct NCursesVisual : public VisualOutput {
private:
	std::size_t UI_Hash = 0;
	bool FlashState = false; ///<The flashing state
	/** @brief Sets the flash state on the screen */
	void SetFlashState(bool State, UserInterface const &UI) {
		FlashState = State;
		if (has_colors()) {
			if (State) Win->FillScreen({(unsigned char)(UI.Color+2),0,0,255});
			else       Win->FillScreen({0,0,0,0}); //FIXME: this should be 2; why does only 1 work?
		} else {
			if (State) Win->FillScreen({1,0,0,255});
			else       Win->FillScreen({0,0,0,0});
		}
	}
	/** @brief Reset the flash timer */
	void reset() {
		FirstTick = std::chrono::steady_clock::now();
		FlashCounter = 1;
	}
	void TriggerUIRedraw() {
		UI_Hash -= 1;
	}
public:
	NCursesVisual(WindowHandle* W) {
		Win = W;
		FirstTick = std::chrono::steady_clock::now();
		LastTick = FirstTick;
		TickTimer = FirstTick;
	}
	virtual ~NCursesVisual() = default;

	/** @brief Draw a flash on the screen */
	virtual void DrawFlash(UserInterface const &UI) override { //FIXME: very sloppy for now; Definitely need to fix how we output to the window;
		if (UI.hash() != UI_Hash) { //Avoid locking the output
			UI_Hash = UI.hash();
			reset();
		} else if (!UI.Flashing) {
			reset();
		}
		typedef std::chrono::milliseconds milliseconds;
		milliseconds Diff = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now() - FirstTick);
		milliseconds DiffTick = std::chrono::duration_cast<milliseconds>(std::chrono::steady_clock::now() - TickTimer);
		long unsigned MillisPerBeat = ComputeMillisecondsPerBeat(UI.BPM);
		long unsigned local_FlashInterval = std::max(std::min(FlashInterval,(long long)MillisPerBeat / 6),(long long)24);
		if ((long unsigned)DiffTick.count() > (long unsigned)(local_FlashInterval)) {
			SetFlashState(false,UI);
		}
		if ((long unsigned)Diff.count() > MillisPerBeat * FlashCounter) {
			SetFlashState(true,UI);
			LastTick = std::chrono::steady_clock::now();
			TickTimer = LastTick;
			FlashCounter += 1;
		}
	}
	virtual void DrawMetronome(UserInterface const &UI) override { Unused(UI);};
	virtual void DrawRaindrops(UserInterface const &UI) override { Unused(UI);};
	virtual void ForceRedraw() override {
		TriggerUIRedraw();
		Win->Redraw();
	}
};

/** @brief NCurses implementation of the drawing functions */
struct NCursesDrawer : public Drawer {
private:
	std::size_t UI_Hash = 0;                                                           ///<The UI hash used to determine whether a draw needs to occur
	std::unique_ptr<ncurses_InputHandler> m_Input;                                     ///<Owning pointer for the input handler
	std::unordered_map<std::string,std::unique_ptr<ncurses_WindowHandle>> m_Children;  ///<A map of window handles
	bool ForceRedraw = false;
	/** Set NCurses color pairs */
	void SetColorPairs() {
		start_color();
		init_pair(2,COLOR_BLUE,COLOR_BLUE); //TODO: UserInput sets a colour scheme; we should make that set the flash colour;
		init_pair(3,COLOR_CYAN,COLOR_CYAN);
		init_pair(4,COLOR_GREEN,COLOR_GREEN);
		init_pair(5,COLOR_YELLOW,COLOR_YELLOW);
		init_pair(6,COLOR_RED,COLOR_RED);
		init_pair(7,COLOR_MAGENTA,COLOR_MAGENTA);
		init_pair(8,COLOR_BLACK,COLOR_BLACK);
		init_pair(9,COLOR_WHITE,COLOR_WHITE);
		init_pair(12,COLOR_BLUE,COLOR_BLACK);
		init_pair(13,COLOR_CYAN,COLOR_BLACK);
		init_pair(14,COLOR_GREEN,COLOR_BLACK);
		init_pair(15,COLOR_YELLOW,COLOR_BLACK);
		init_pair(16,COLOR_RED,COLOR_BLACK);
		init_pair(17,COLOR_MAGENTA,COLOR_BLACK);
		init_pair(18,COLOR_BLACK,COLOR_BLACK);
		init_pair(19,COLOR_WHITE,COLOR_BLACK);
	}
	void TriggerUIRedraw() {
		UI_Hash -= 1;
	}
public:
	NCursesDrawer() {
		initscr();
		noraw();
		echo();
		cbreak();
		curs_set(0);
		noqiflush();
		keypad(stdscr,true);
		timeout(2);
		SetColorPairs();
		Refresh();
		m_Input = std::make_unique<ncurses_InputHandler>(ncurses_InputHandler(stdscr));
	}

	virtual ~NCursesDrawer() {
		m_Children.clear();
		endwin();
	}

	/** Redraw everything on screen */
	virtual void Redraw() override {
		for (auto &Window : m_Children) {
			Window.second->Redraw();
			Window.second->Refresh();
		}
		Refresh();
	}

	/** Refresh all windows */
	virtual void Refresh() override {
		for (auto &Window : m_Children) {
			Window.second->Refresh();
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
		if (NewUI == UI_Hash && !ForceRedraw) {return;}
		else {UI_Hash = NewUI;}
		WINDOW* tUI = m_Children["InputWindow"]->GetHandle();
		wclear(tUI);
		box(tUI,0,0);
		int nLabels = UI.NumberOfLabels();
		for (int i = 0; i != nLabels; i++) {
			if (i == UI.CurrentSelection) {wattrset(tUI,A_STANDOUT);}
			mvwprintw(tUI,i+1,1,"%s",UI.GetLabel(i).c_str());
			wattrset(tUI,A_NORMAL);
			wprintw(tUI,"      ");
		}
	}
	
	/** Update the visuals */
	virtual void UpdateVisual(UserInterface const &UI) override {
		if (ForceRedraw) {
			m_VOut->ForceRedraw();
		}
		if (!m_VOut) return;
		m_VOut->DrawFlash(UI);
		m_VOut->DrawMetronome(UI);
		m_VOut->DrawRaindrops(UI);
	}

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
		m_VOut = std::make_unique<NCursesVisual>(NCursesVisual(m_Children["VisualWindow"].get()));
	}

	/** Process a resize event */
	void ProcessResize() {
		BoxSize<int> const WinSize = GetWindowSize();
		switch (Orientation) {
		case Location::North: {
			m_Children.at("InputWindow")->resize(7,WinSize.X);
			m_Children.at("VisualWindow")->resize(WinSize.Y-7,WinSize.X);
			break;
		}
		}
		TriggerUIRedraw();
		Redraw();
	}

	/** Implementation of local input handler */
	void HandleInput(FullInput const &Interaction) {
		ForceRedraw = false;
		if (Interaction.Keypress == KEY_RESIZE) {
			ForceRedraw = true;
			ProcessResize();
			Refresh();
		}
	}
};

/** @brief NCurses implementation of the input pipe */
struct ncurses_InputPipe : public PipeInputToUI {
private:
	/** @brief Handle the user "selection" key */
	void HandleSelectionKey(UserInterface &UI) {
		switch ((UserInterface::Selection)(UI.CurrentSelection)) {
		case UserInterface::Selection::TIMESIGNATURE: break;                  //need to create window
		case UserInterface::Selection::BEATSPERMIN: break;                    //need to create window
		case UserInterface::Selection::COLORSEL: break;                       //Not applicable
		case UserInterface::Selection::VISUALIZATION: break;                  //Not applicable
		case UserInterface::Selection::FLASHING: UI.ToggleFlashing(); break;
		default: break;
		}
	}

	/** @brief Handle the user's keyboard arrow key input */
	void HandleArrowKey(UserInterface &UI, char Direction) {
		switch ((UserInterface::Selection)(UI.CurrentSelection)) {
		case UserInterface::Selection::TIMESIGNATURE: break; //N/A
		case UserInterface::Selection::BEATSPERMIN:   //Increment BPM
			UI.BPM += (Direction > 0) - (UI.BPM > 1.0f && Direction < 0); 
			break;
		case UserInterface::Selection::COLORSEL:      //Increment color
			UI.SetColor((Direction > 0) - (Direction < 0));
			break;
		case UserInterface::Selection::VISUALIZATION: //Increment visualization
			UI.SetVisualization(Direction);
			break;
		case UserInterface::Selection::FLASHING:      //Increment flashing
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
