#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <algorithm>
#include <cmath>

typedef unsigned char ColorType;

/** @brief North/south/east/west enum */
enum class Location {
	North = 0, ///<Top of the screen
	East,      ///<Right of the screen
	South,     ///<Bottom of the screen
	West       ///<Left of the screen
};

template <typename NumberType = int>
struct BoxSize {
	NumberType X;
	NumberType Y;
};

template <typename NumberType = int>
inline bool operator!=(BoxSize<NumberType>& A, BoxSize<NumberType>& B) {
	return (A.X != B.X) && (A.Y != B.Y);
}
template <typename NumberType = int>
inline bool operator==(BoxSize<NumberType>& A, BoxSize<NumberType>& B) {
	return (A.X == B.X) && (A.Y == B.Y);
}

/** @brief A list of (hopefully to-be-added) visualizations */
enum class Visualization : unsigned char {
	Pendulum,
	ParticlesTopDown,
	ParticlesBottomUp,
	ParticlesLeftToRight,
	ParticlesRightToLeft,
	ProgressTopDown,
	ProgressBottomUp,
	ProgressLeftToRight,
	ProgressRightToLeft,
	FlashOnly
};

/** @brief A list of colours */
struct ColorPair {
	ColorType FG;
	ColorType BG;
	ColorType TXT;
};

/** @brief Time signature */
struct Signature {
	unsigned char upper;
	unsigned char lower;
};

/** @brief Container for window size in x- and y- directions */
struct WindowSizeContainer {
	int X;
	int Y;
};

/** @brief Container for a floating point location */
template <typename LengthType = float>
struct LocationContainer {
	LengthType X;
	LengthType Y;
	bool operator!=(LocationContainer const &a) {
		return (a.X != X || a.Y != Y);
	}
	bool operator==(LocationContainer const &a) {
		return (a.X == X && a.Y == Y);
	}
};

/** @brief A vector of some length starting at coordinates (0,0) and ending at end point (X,Y) */
struct LengthVector {
	float X;
	float Y;
	LengthVector(LocationContainer<float> const &Start, LocationContainer<float> const &End) :
		X(End.X - Start.X),
		Y(End.Y - Start.Y) {}
	LengthVector Normal() const {
		LengthVector ret(*this);
		std::swap(ret.X,ret.Y);
		ret.Y = -ret.Y;
		return ret;
	}
	void Normalize() {
		if (Length() == 0) {
			X = 0;
			Y = 0;
		} else {
			X /= Length();
			Y /= Length();
		}

	}
	float Length() const {
		return sqrt(X*X + Y*Y);
	}
	LengthVector operator*(float Factor) {
		X *= Factor;
		Y *= Factor;
		return *this;
	}
	LengthVector operator/(float Factor) {
		X *= Factor;
		Y *= Factor;
		return *this;
	}
};

/** @brief A structure containing the inputs for controlling the visualization */
struct UserInputs {
	Signature TimeSignature {4,4};
	float BPM {120};
	ColorType ColorScheme {0};
	Visualization Vis {Visualization::FlashOnly};
	bool Flashing {false};
};

#endif //TYPES_HPP_
