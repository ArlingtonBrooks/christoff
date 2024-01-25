#ifndef INTERFACE_HPP_
#define INTERFACE_HPP_

#include "Types.hpp"

#include <array>      //array
#include <functional> //hash
#include <iomanip>    //precision
#include <sstream>    //stringstream
#include <string>     //string

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
	const int MaxColors = 10;                     ///<Maximum number of colour patterns
	std::array<std::string,5> Labels;             ///<Array of output labels
	int CurrentSelection = 0;                     ///<Currently selected field
	float BPM = 120;                              ///<Beats per minute field
	int Color = 0;                                ///<Color field
	short Signature_Upper = 4;                    ///<Time signature upper field
	short Signature_Lower = 4;                    ///<time signature lower field
	unsigned char VisualizationType = 0;          ///<Selected visualization
	bool Flashing = false;                        ///<Whether to flash the screen at intervals

	void MoveSelection(char direction) {
		if (direction > 0) {
			if (CurrentSelection < 4) CurrentSelection += 1;
			else CurrentSelection = 0;
		} else if (direction < 0) {
			if (CurrentSelection > 0) CurrentSelection -= 1;
			else CurrentSelection = 4;
		}
	}
	void SetBPM(float newBPM) {BPM = newBPM;}
	void SetColor(char direction) {
		if (direction > 0) {
			if (Color < MaxColors) Color += 1;
			else Color = 0;
		} else if (direction < 0) {
			if (Color > 0) Color -= 1;
			else Color = MaxColors;
		}
	}
	void SetSignature(short upper, short lower) {Signature_Upper = upper; Signature_Lower = lower;}
	void SetVisualization(char direction) {
		if (direction > 0) {
			if (VisualizationType < MaxVisualizations) VisualizationType += 1;
			else VisualizationType = 0;
		} else if (direction < 0) {
			if (VisualizationType > 0) VisualizationType -= 1;
			else VisualizationType = MaxVisualizations;
		}
	}
	void ToggleFlashing() {Flashing = !Flashing;}
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
	std::size_t hash() {
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
};


/** @brief Basic class for drawing windows to screen */
struct Drawer {
	Location Orientation = Location::North; ///<Orientation of the user interface
	void SetOrientation(Location L) {
		Orientation = L;
	}
	static constexpr bool IsDrawerType() {return true;}
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

/** @brief User input handling */
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
	UserInterface m_UI;
	WindowSystem m_WS;
	InputSystem m_Input;
public:
	MainWindow() {
		m_WS.CreateInputWindow();
		m_WS.CreateVisualWindow();
		Refresh();
	}
	
	void Refresh() {
		m_WS.Refresh();
	}

	void Redraw() {
		m_WS.Redraw();
		m_WS.Refresh();
	}

	void Draw() {
		PrintUI();
		UpdateVisual();
	}

	void PrintUI() {
		m_WS.PrintUI(m_UI);
	}

	void UpdateVisual() {
		m_WS.UpdateVisual(m_UI);
	}

	void HandleInput(bool &Running) {
		int Keypress = m_Input.Keyboard(m_UI);
		if (Keypress == 'q') Running = false; //exit key
	}
};

#endif
