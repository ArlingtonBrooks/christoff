#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <algorithm>
#include <cmath>
#include <type_traits>

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

template <typename NumberType>
using Position = BoxSize<NumberType>;

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

/** @brief A container for colors */
template <typename Base>
struct ColorType {
	Base R;
	Base G;
	Base B;
	Base A;
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
template <typename NumberType = float>
struct LengthVector {
	static_assert(std::is_floating_point<NumberType>::value);
	NumberType X;
	NumberType Y;
	LengthVector(LocationContainer<NumberType> const &Start, LocationContainer<NumberType> const &End) :
		X(End.X - Start.X),
		Y(End.Y - Start.Y) {}
	LengthVector(Position<NumberType> const &Start, Position<NumberType> const &End) :
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
	NumberType Length() const {
		return sqrt(X*X + Y*Y);
	}
	/** @brief Return the closest point as a factor of the original vector */
	NumberType ClosestPoint(Position<NumberType> P) const {
		NumberType A = (P.X * X + P.Y * Y) / (X*X + Y*Y);
		return A;
	}
	LengthVector operator*(NumberType Factor) {
		X *= Factor;
		Y *= Factor;
		return *this;
	}
	LengthVector operator/(NumberType Factor) {
		X *= Factor;
		Y *= Factor;
		return *this;
	}
};

/** A basic circle */
template <typename NumberType = float>
struct Circle {
	NumberType Radius;
	/** @brief Check if a location relative to the center is inside the radius */
	bool IsInside(Position<NumberType> const &P) {
		return (P.X * P.X + P.Y * P.Y) < (Radius * Radius);
	}
	/** @brief Return area */
	NumberType Area() {
		static const double Pi = acos(-1.0);
		return Pi * Radius * Radius;
	}
};

/** A basic triangle */
template <typename NumberType = float>
struct Triangle {
	Position<NumberType> Pt1;
	Position<NumberType> Pt2;
	Position<NumberType> Pt3;
	/** @brief Check if a location relative to the center is inside the radius */
	bool IsInside(Position<NumberType> const &P) {
		NumberType Len1 = std::pow((Pt1.X - Pt2.X),2) + std::pow((Pt1.Y - Pt2.Y),2);
		NumberType Len2 = std::pow((Pt3.X - Pt2.X),2) + std::pow((Pt3.Y - Pt2.Y),2);
		NumberType Len3 = std::pow((Pt1.X - Pt3.X),2) + std::pow((Pt1.Y - Pt3.Y),2);

		NumberType PLen1 = std::pow(P.X - Pt1.X,2) + std::pow(P.Y - Pt1.Y,2);
		NumberType PLen2 = std::pow(P.X - Pt2.X,2) + std::pow(P.Y - Pt2.Y,2);
		NumberType PLen3 = std::pow(P.X - Pt3.X,2) + std::pow(P.Y - Pt3.Y,2);

		NumberType LenMax = std::max(std::max(Len1,Len2),Len3);
		NumberType PLenMax = std::max(std::max(PLen1,PLen2),PLen3);

		return PLenMax > LenMax;
	}
	/** @brief Return area */
	NumberType Area() {
		throw 1; //not yet implemented;
	}
};

/** A basic line */
template <typename NumberType = float>
struct Line {
	Position<NumberType> Pt1;
	Position<NumberType> Pt2;
	/** @brief Check if a position is on the line segment within some tolerance */
	bool IsCoincident(Position<NumberType> const &P, NumberType Tolerance) {
		//Construct a vector;
		LengthVector<NumberType> LV(Pt1,Pt2);
		NumberType Closest = LV.ClosestPoint();
		if (Closest <= 1.0+Tolerance && Closest >= 0.0-Tolerance) {
			LV = LV * Closest;
			NumberType Distance = sqrt(std::pow((LV.X - P.X),2) + std::pow((LV.Y - P.Y),2));
			return Distance <= Tolerance;
		} else {
			return false;
		}
	}
};

/** A basic rectangle */
template <typename NumberType = float>
struct Rect {
public:
	Position<NumberType> Pt1;
	Position<NumberType> Pt2;
	/** @brief Check if a position is on the line segment within some tolerance */
	bool IsInside(Position<NumberType> const &P) {
		bool IsBetween = true;
		if (Pt1.X > Pt2.X) { IsBetween &= (P.X <= Pt1.X && P.X >= Pt2.X);}
		if (Pt1.X <= Pt2.X) { IsBetween &= (P.X >= Pt1.X && P.X <= Pt2.X);}
		if (Pt1.Y > Pt2.Y) { IsBetween &= (P.Y <= Pt1.Y && P.Y >= Pt2.Y);}
		if (Pt1.Y <= Pt2.Y) { IsBetween &= (P.Y >= Pt1.Y && P.Y <= Pt2.Y);}
		return IsBetween;
	}
};

/** @brief A structure containing the inputs for controlling the visualization */
struct UserInputs {
	Signature TimeSignature {4,4};
	float BPM {120};
	unsigned char ColorScheme {0};
	Visualization Vis {Visualization::FlashOnly};
	bool Flashing {false};
};

#endif //TYPES_HPP_
