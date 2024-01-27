#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_

#include "Types.hpp"

#include <array>      //array
#include <chrono>     //std::chrono
#include <functional> //hash
#include <iomanip>    //precision
#include <memory>     //unique_ptr
#include <sstream>    //stringstream
#include <string>     //string
#include <stdexcept>  //exceptions

const long long FlashInterval = 64; //milliseconds to flash up on screen;

/** @brief User input interface */
struct UserInterface {
	/** @enum Integers defining interface selection */
	enum class Selection : unsigned {
		TIMESIGNATURE = 0,
		BEATSPERMIN = 1,
		COLORSEL = 2,
		VISUALIZATION = 3,
		FLASHING = 4
	};
	const unsigned char MaxVisualizations = 10;   ///<Maximum number to display for visualizations
	const int MaxColors = 7;                      ///<Maximum number of colour patterns
	std::array<std::string,5> Labels;             ///<Array of output labels
	int CurrentSelection = 0;                     ///<Currently selected field
	float BPM = 120;                              ///<Beats per minute field
	int Color = 0;                                ///<Color field
	short Signature_Upper = 4;                    ///<Time signature upper field
	short Signature_Lower = 4;                    ///<time signature lower field
	unsigned char VisualizationType = 0;          ///<Selected visualization
	bool Flashing = false;                        ///<Whether to flash the screen at intervals

	/** @brief UI Element selector (change CurrentSelection based on input) */
	void MoveSelection(char direction) {
		if (direction > 0) {
			if (CurrentSelection < 4) CurrentSelection += 1;
			else CurrentSelection = 0;
		} else if (direction < 0) {
			if (CurrentSelection > 0) CurrentSelection -= 1;
			else CurrentSelection = 4;
		}
	}
	/** @brief Set the BPM value */
	void SetBPM(float newBPM) {BPM = std::max(0.01f,std::min(newBPM,350.0f));}

	/** @brief UI Color selection (change Color based on input) */
	void SetColor(char direction) {
		if (direction > 0) {
			if (Color < MaxColors) Color += 1;
			else Color = 0;
		} else if (direction < 0) {
			if (Color > 0) Color -= 1;
			else Color = MaxColors;
		}
	}

	/** @brief Set the time signature based on inputs */
	void SetSignature(short upper, short lower) {Signature_Upper = upper; Signature_Lower = lower;}

	/** @brief UI Visualization selection (change visualization based on input) */
	void SetVisualization(char direction) {
		if (direction > 0) {
			if (VisualizationType < MaxVisualizations) VisualizationType += 1;
			else VisualizationType = 0;
		} else if (direction < 0) {
			if (VisualizationType > 0) VisualizationType -= 1;
			else VisualizationType = MaxVisualizations;
		}
	}

	/** @brief Set flashing selection (inverse of what is currently selected) */
	void ToggleFlashing() {Flashing = !Flashing;}

	/** @brief Returns the number of labels available (this MUST be updated if options are added to the UI) */
	constexpr int NumberOfLabels() {return 5;}

	/** @brief Rewrites label strings */
	std::string GetLabel(int index) {
		Selection Sel = (Selection)(index);
		std::stringstream Str;
		if (Sel == Selection::TIMESIGNATURE) {
			Str << "Time signature: " << Signature_Upper << " : " << Signature_Lower;
		} else if (Sel == Selection::BEATSPERMIN) {
			Str << std::setprecision(5) <<  "Beats Per Minute: " << BPM;
		} else if (Sel == Selection::COLORSEL) {
			Str << "Color scheme: " << Color;
		} else if (Sel == Selection::VISUALIZATION) {
			Str << "Visualization: " << (unsigned)VisualizationType;
		} else if (Sel == Selection::FLASHING) {
			Str << "Flashing: ";
			if (!Flashing) {Str << "No";}
			else {Str << "Yes";}
		}
		Labels[index] = Str.str();
		return Labels[index];
	}

	/** @brief Computes a "hash" of the user input selections */
	std::size_t hash() const {
		std::size_t hash = 99194853094755497;
		std::size_t I = std::hash<int>()(CurrentSelection);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<float>()(BPM);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<int>()(Color);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<short>()(Signature_Upper);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<short>()(Signature_Lower);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<unsigned char>()(VisualizationType);
		hash = ((hash << 5) + hash) + I;
		I = std::hash<bool>()(Flashing);
		hash = ((hash << 5) + hash) + I;
		return hash;
	}
};

/** @brief InputHandler interface */
struct InputHandler {
	constexpr bool IsInputHandlerType() {return true;}
	/** @brief Receive a character from the keyboard */
	virtual int GetCharInput() = 0;
	/** @brief Receive a string from the keyboard (eg: an input textbox) */
	virtual std::string GetStringInput() = 0;
};

/** @brief An interface definition for window objects */
struct WindowHandle {
	static constexpr bool IsWindowType() {return true;}

	/** @brief Return whether this window is currently active/drawing */
	virtual bool IsActive() const = 0;
	/** @brief Refresh the window */
	virtual void refresh() = 0;
	/** @brief Force redrawing all elements on the window */
	virtual void redraw() = 0;
	/** @brief Get the size of the window */
	virtual BoxSize<int> GetSize() = 0;

	/* Primitives */
	/** @brief Draw a circle of radius at a location with a border color, thickness, and optional fill */
	virtual void DrawCircle(float Radius, Position<float> const &Loc, ColorType<unsigned char> Border, float BorderThickness, bool Fill = false, ColorType<unsigned char> FillColor = {0,0,0,0}) = 0;
	/** @brief Draw a triangle based on 3 points with a border, thickness, offset, and fill options */
	virtual void DrawTriangle(Position<float> const &Pt1, Position<float> const &Pt2, Position<float> const &Pt3, ColorType<unsigned char> Border, float BorderThickness, Position<float> const &Offset = {0,0}, bool Fill = false, ColorType<unsigned char> FillColor = {0,0,0,0}) = 0;
	/** @brief Draw a line between two points with a border, thickness, and offset */
	virtual void DrawLine(Position<float> const &Pt1, Position<float> const &Pt2, float Thickness, Position<float> const &Offset = {0,0}) = 0;
	/** @brief Fill entire screenn with a colour */
	virtual void FillScreen(ColorType<unsigned char> FillColor) = 0;
};

/** @brief Basic class for drawing visualizations to screen */
struct VisualOutput {
protected:
	std::chrono::time_point<std::chrono::steady_clock> FirstTick; ///<The first metronome tick time
	std::chrono::time_point<std::chrono::steady_clock> LastTick;  ///<The last time the metronome ticked
	std::chrono::time_point<std::chrono::steady_clock> TickTimer; ///<A timer used to control the amount of time a 'flash' is on screen
	long long FlashCounter = 0;                                   ///<A counter for the number of times the metronome has ticked
public:
	WindowHandle *Win;                                            ///<Non-owning pointer to a window;
	static constexpr bool IsVisualType() {return true;}           ///<Returns that any derived classes are of visual type (guaranteeing certain draw options)
	virtual void DrawFlash(UserInterface const &UI) = 0;          ///<Draw the flash visualization
	virtual void DrawMetronome(UserInterface const &UI) = 0;      ///<Draw the metronome visualization
	virtual void DrawRaindrops(UserInterface const &UI) = 0;      ///<Draw the raindrops visualization
};

/** @brief Basic class for drawing windows to screen */
struct Drawer {
protected:
	std::unique_ptr<VisualOutput> m_VOut = nullptr;               ///<Visual output owning pointer (should be contained within the drawer)
public:
	Location Orientation = Location::North;                       ///<Orientation of the user interface (TODO: this should be configurable?)
	void SetOrientation(Location L) {
		Orientation = L;
	}
	static constexpr bool IsDrawerType() {return true;}           ///<Returns that any derived classes are of Drawer type (guaranteeing certain functions)
	/** @brief Redraw all elements on the window */
	virtual void Redraw() = 0;
	/** @brief Refresh the window */
	virtual void Refresh() = 0;
	/** @brief Get the size of the current window */
	virtual BoxSize<int> GetWindowSize() = 0;
	/** @brief Print the user interface to the screen */
	virtual void PrintUI(UserInterface &UI) = 0;
	/** @brief Update the visuals on the screen */
	virtual void UpdateVisual(UserInterface const &UI) = 0;
	/** @brief Create the user input section of the screen */
	virtual void CreateInputWindow() = 0;
	/** @brief Create the visuals section of the screen */
	virtual void CreateVisualWindow() = 0;
};

/** @brief User input handling 
 * This class acts as an interface between the output system (be it ncurses, opengl, webgui, etc) and the UserInterface class allowing any arbitrary input to be translated to something that can modify the UserInterface options
 */
struct PipeInputToUI {
	static constexpr bool IsInputHandler() {return true;}
	/** @brief Pipe user's keyboard input to the user interface */
	virtual int Keyboard(UserInterface &UI) = 0;
	//note: may add mouse events in future
};

/** 
 * @brief Main Window class which handles user interaction with program and its elements 
 * @note Since this is the top level, it doesn't need to know about ncurses or the drawing method
 */
template <typename WindowSystem, typename InputSystem>
class MainWindow {
	static_assert(WindowSystem::IsDrawerType(),"Window system must be derived from Drawer type");
	static_assert(InputSystem::IsInputHandler(),"Input system must be derived from PipeInputToUI type");
private:
	UserInterface m_UI;                                ///<The user interface
	WindowSystem m_WS;                                 ///<The window system to be used for output
	InputSystem m_Input;                               ///<The system by which input is captured
public:
	MainWindow() {
		m_WS.CreateInputWindow();
		m_WS.CreateVisualWindow();
		Refresh();
	}
	
	/** @brief Refresh the screen without necessarily redrawing everything */
	void Refresh() {
		m_WS.Refresh();
	}

	/** @brief Force redraw everything on screen */
	void Redraw() {
		m_WS.Redraw();
		m_WS.Refresh();
	}

	/** @brief Draw the screen in its current state */
	void Draw() {
		PrintUI();
		UpdateVisual();
	}

	/** @brief Print the UI in its current state */
	void PrintUI() {
		m_WS.PrintUI(m_UI);
	}

	/** @brief Update the visual output */
	void UpdateVisual() {
		m_WS.UpdateVisual(m_UI);
	}

	/** @brief Get input from the input system. 
	 * TODO: In the future, we may need to include parser for mouse, midi, or other options
	 */
	void HandleInput(bool &Running) {
		int Keypress = m_Input.Keyboard(m_UI);
		if (Keypress == 'q') Running = false; //exit key
	}
};

#endif
