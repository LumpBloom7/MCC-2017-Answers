#include <string>
#include <ostream>

//This code written and owned by Terry Richards.
//terryrichards999@gmail.com

//You may use this code for personal use only. 

//For commercial use, please contact me and we
//can discuss the matter.

//For the purposes of the above statements,
//"commercial use" is any use of my code in a
//product that you sell or otherwise realise 
//financial gain from. It does not include, for
//example, writing a tool that you use to help
//you with your work as long as that tool is not
//subsequently included in any commercial product.
//"Personal use" is any use that is not commercial!

//Version 1.1.1

//Change log - see Readme.txt

namespace TRS{

#if defined (_WIN64)

#define XI_WORD UINT64
#define XI_MAX 1000000000000000000
#define XI_HALF_MAX 1000000000
#define XI_DECS_PER_WORD 18
	typedef __int64 XI_IX;
#define XI_IX_MAX _I64_MAX

#elif defined (_WIN32)

#define XI_WORD UINT32
#define XI_MAX 100000000
#define XI_HALF_MAX 10000
#define XI_DECS_PER_WORD 8
	typedef int XI_IX;
#define XI_IX_MAX INT_MAX

	//To use XInt on another platform, you will need to create an
	//#elif block here. See the included text file for the 
	//explanation of these parameters. 
	//If you do create your own block, I would appreciate it if
	//you could send me a copy and I will merge it into the base 
	//code.

#endif

	//Some useful constants. Let me know if you need any others.
	//They are currently specified to 40 decimal places. Let me
	//know if you need more.

	//You create an XInt with these values this way:
	//	XInt E(XI_E);
	//	XInt Pi(XI_PI);

#define XI_E		"2.7182818284590452353602874713526624977572"
#define XI_HALF_PI	"1.5707963267948966192313216916397514420986"
#define XI_PI		"3.1415926535897932384626433832795028841972"
#define XI_TWO_PI	"6.2831853071795864769252867665590057683944"

	//Comparison results

	enum XI_Comp{

		Compare_EQ,	
		Compare_GT,	
		Compare_LT

	};

	//Rounding types and decimal places

	enum XI_RoundingType{

		RT_Truncate,		//Normal for integers - symetrical
		RT_Positive,		//Round towards +infinity - asymetrical
		RT_Negative,		//Round towards -infinity - asymetrical
		RT_TowardsZero,		//The same as Truncate - symetrical
		RT_AwayFromZero,	//Symetrical
		RT_Nearest			//The one you learned in school. 

	};

	enum XI_RoundingFive{

		//NOTE: This is only used if the main rounding type is 
		//RT_Nearest. For all the other types, 5 is not a special
		//case and is treated like all the other non-zero digits

		RT5_Positive,		//Round towards +infinity - asymetrical
		RT5_Negative,		//Round towards -infinity - asymetrical
		RT5_TowardsZero,	//Symetrical
		RT5_AwayFromZero,	//The one you learned in school - symetrical
		RT5_Even,			//Banker's rounding - symetrical
		RT5_Odd				//Rarely used because it can never round to zero. Symetrical

	};

	//When decimal places are specified, it must be zero or a positive number.
	//These "magic" values are used to let the code calculate a suitable number 
	//of places itself.

#define DP_LHS  -1 //Use decimal places of left argument
#define DP_RHS  -2 //Use decimal places of right argument
#define DP_SUM  -3 //Use decimal places of right and left added
#define DP_MUL  -4 //Use decimal places of right and left multiplied

	//The following parameter controls the memory management scheme. It is a
	//classic space v. speed trade-off. The higher it is, the less often memory
	//will have to be allocated and copied. The lower it is, the less memory
	//will be used. It can be as low as zero in which case memory will only
	//be allocated as it is used.

#define XI_EXTRA_WORDS 20

class XInt{

	protected:

		XI_WORD*	m_pWords;
		XI_IX		m_NumWords;
		XI_IX		m_AllocatedWords;
		XI_IX		m_Exponent; //Can be + or - inside the class. For returned values, it is always 0 or negative.
		bool		m_Negative;
		bool		m_Infinite;
		bool		m_NaN;

		void		Rationalise();
		void		RationaliseWords();
		inline void	GrowTo(XI_IX NumWords){ GrowBy(NumWords - m_NumWords); };
		void		GrowBy(XI_IX NumWords);
		void		AddWords(const XInt& Other);
		void		SubtractWords(const XInt& Other);
		void		Unpack(XI_WORD* pA, XI_IX NumHalfWords) const;
		void		Pack(XI_WORD* pA, XI_IX NumHalfWords);
		void		Div2();
		void		Div10(XI_RoundingType RT, XI_RoundingFive R5);
		XInt		Exponentiate(const XInt& Exp) const;
		static void	Align(XInt& A, XInt& B);
		void		NumToString(std::string& str, XI_IX Base) const;
		void		Base10ToString(std::string& str) const;
		void		ResolveDP(const XInt& Other, XI_IX& Dec) const;
		void		ResolveDP(XI_IX Dec) const;
		XI_Comp		Compare(const XInt& Other) const;
		XInt		DoArcTan(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		inline XInt	RadToDeg(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return (*this * XInt(360l)).Divide(XInt(XI_TWO_PI), NumDecimals, RT, R5); };
		inline XInt	DegToRad(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return (*this * XInt(XI_TWO_PI)).Divide(XInt(360l), NumDecimals, RT, R5); };

	public:

		XInt(XI_IX Val = 0, XI_IX Decimals = 0);
		XInt(const XInt* pOther);
		XInt(const XInt& Other) :XInt(&Other){};
		XInt(const std::string str);
		~XInt();

		//Actions

		void		MakeInfinite();
		void		MakeNaN();
		void		MakeZero();
		void		Flip(){ if (!IsZero() && !m_NaN) m_Negative = !m_Negative; };
		void		Round(XI_IX Position = 0, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero); //Index[0] is the first digit left of the decimal place
		void		SetDecimals(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero);
		void		CopySign(const XInt& Other){ if (!IsZero() && !m_NaN) m_Negative = Other.m_Negative; };

		//Query functions

		bool		IsZero() const;
		bool		IsInfinite() const{ return m_Infinite; };
		bool		IsNegative() const{ return m_Negative; };
		bool		IsNaN() const{ return m_NaN; };
		bool		IsNumber() const{ return !IsInfinite() && !IsNaN(); };
		XInt		Abs() const;
		XInt		IntPart() const; //Returns everything to the left of the decimal point
		XInt		FracPart() const; //Returns everything to the right of the decimal point
		XI_WORD		operator [](const XI_IX Index) const; //Returns the decimal digit at an Index. Index[0] is the first digit left of the decimal place
		XI_IX		NumSignificantDigits() const;

		//Arithmetic & Math functions

		XInt		Add(const XInt& Other) const;
		XInt		Subtract(const XInt& Other) const{return Add(-Other);};
		XInt		Multiply(const XInt& Other) const;
		XInt		Divide(const XInt& Other, XI_IX Dec = DP_LHS, XI_RoundingType RT = RT_Truncate, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		Pow(const XInt Exp) const;
		XInt		Root(const XInt N) const;
		XInt		Log(const XInt& Base = 2, XI_IX NumDecimals = 30) const;
		XInt		AntiLog(const XInt& Base = 2, XI_IX NumDecimals = 30) const;
		XInt		Sin(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		Cos(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		Tan(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		ArcSin(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		ArcCos(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		ArcTan(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const;
		XInt		DSin(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return DegToRad(NumDecimals, RT, R5).Sin(NumDecimals, RT, R5); };
		XInt		DCos(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return DegToRad(NumDecimals, RT, R5).Cos(NumDecimals, RT, R5); };
		XInt		DTan(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return DegToRad(NumDecimals, RT, R5).Tan(NumDecimals, RT, R5); };
		XInt		DArcSin(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return ArcSin(NumDecimals, RT, R5).RadToDeg(NumDecimals, RT, R5); };
		XInt		DArcCos(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return ArcCos(NumDecimals, RT, R5).RadToDeg(NumDecimals, RT, R5); };
		XInt		DArcTan(XI_IX NumDecimals, XI_RoundingType RT = RT_Nearest, XI_RoundingFive R5 = RT5_AwayFromZero) const { return ArcTan(NumDecimals, RT, R5).RadToDeg(NumDecimals, RT, R5); };

		//Arithmetic operators

		//Note: The following overloads are mainly provided for compatability with existing code.
		//For new code, it is preferable to use the native functions provided by the class as they
		//give you more control (in some cases) whereas these use (hopefully reasonable) defaults.

		XInt&		operator =(const XInt& Other);
		XInt		operator -() const{ XInt Work(this); Work.Flip(); return Work; };
		XInt		operator +(const XInt& Other) const{ return Add(Other); };
		XInt&		operator +=(const XInt& Other){ *this = Add(Other); return *this; };
		XInt		operator -(const XInt& Other) const{ return Add(-Other); };
		XInt&		operator -=(const XInt& Other){ *this = Add(-Other); return *this; };
		XInt		operator *(const XInt& Other) const{ return Multiply(Other); };
		XInt&		operator *=(const XInt& Other){ *this = Multiply(Other); return *this; };
		XInt		operator /(const XInt& Other) const{ return Divide(Other); };
		XInt&		operator /=(const XInt& Other){ *this = Divide(Other); return *this; };
		XInt		operator %(const XInt N) const;
		XInt&		operator %=(const XInt N){ *this = *this % N; return *this; };
		XInt&		operator ++(){ *this = Add(XInt(1)); return *this; };				//++x
		XInt&		operator --(){ *this = Add(XInt(-1)); return *this; };				//--x
		XInt		operator ++(int){ XInt tmp = *this; Add(XInt(1)); return tmp; };	//x++
		XInt		operator --(int){ XInt tmp = *this; Add(XInt(-1)); return tmp; };	//x--

		//Bit operators

		XInt		operator ~() const;
		XInt		operator |(const XInt& Other) const;
		XInt&		operator |=(const XInt& Other){ *this = *this | Other; return *this; };
		XInt		operator &(const XInt& Other) const;
		XInt&		operator &=(const XInt& Other){ *this = *this & Other; return *this; };
		XInt		operator ^(const XInt& Other) const;
		XInt&		operator ^=(const XInt& Other){ *this = *this ^ Other; return *this; };
		XInt		operator >>(const XI_IX Places) const;
		XInt		operator >>=(const XI_IX Places){ *this = *this >> Places; return this; };
		XInt		operator <<(const XI_IX Places) const;
		XInt		operator <<=(const XI_IX Places){ *this = *this << Places; return this; };

		//Comparison operators

		bool		operator ==(const XInt& Other) const{ return Compare(Other) == Compare_EQ; };
		bool		operator !=(const XInt& Other) const{ return Compare(Other) != Compare_EQ; };
		bool		operator  <(const XInt& Other) const{ return Compare(Other) == Compare_LT; };
		bool		operator  >(const XInt& Other) const{ return Compare(Other) == Compare_GT; };
		bool		operator <=(const XInt& Other) const{ return Compare(Other) != Compare_GT; };
		bool		operator >=(const XInt& Other) const{ return Compare(Other) != Compare_LT; };

		//Streaming

		void		StreamOut(std::ostream& os) const;
		void		StreamIn(std::istream& is);

		//Conversion operators

#ifdef _AFX
		void		operator = (const CString& Source);
		operator CString () const;
#endif
		void		operator = (const std::string& Source);
		operator std::string() const;
		void		operator = (const XI_IX Source);
		operator XI_IX() const;
		void		operator = (const long double Source);
		operator long double() const;
		void		operator = (bool Source){ if (Source) *this = XI_IX(1); else *this = XI_IX(0); };
		operator bool() const { return !IsZero(); };

};

	//Note: The following overloads are mainly provided for compatability with existing code.
	//For new code, it is preferable to use the native functions provided by the class as they
	//give you more control (in some cases) whereas these use (hopefully reasonable) defaults.
	//Any functions not specifically overloaded here (e.g. the less common trig functions) can still be 
	//used via the long double conversion operators. However, this may result in loss of precision
	//and/or overflow conditions. I am currently working on implementing these functions natively.

	//Math.h overloads - not declared as friends because they only use public interfaces

	inline XInt abs(const XInt A){ return A.Abs(); };
	inline XInt acos(const XInt A){ return A.ArcCos(10); };
	inline XInt asin(const XInt A){ return A.ArcSin(10); };
	inline XInt atan(const XInt A){ return A.ArcTan(10); };
	inline XInt atan2(const XInt A, const XInt B){ return B.Divide(A,10).ArcTan(10); };
	inline XInt cbrt(const XInt A){ return A.Root(XInt(3l)); };
	inline XInt ceil(const XInt A){ XInt B(A); B.Round(0l, RT_Positive); return B; };
	inline XInt copysign(const XInt A, const XInt B){ XInt C(A); C.CopySign(B); return C; };
	inline XInt cos(const XInt A){ return A.Cos(10); };
	inline XInt exp(const XInt A){ return A.AntiLog(XInt(XI_E)); };
	inline XInt exp2(const XInt A){ return A.AntiLog(XInt(2l)); };
	inline XInt expm1(const XInt A){ return A.AntiLog(XInt(XI_E)) - XInt(1l); };
	inline XInt fabs(const XInt A){ return A.Abs(); };
	inline XInt fdim(const XInt A, const XInt B){ if (A > B) return A - B; return XInt((XI_IX)0); };
	inline XInt floor(const XInt A){ XInt B(A); B.Round(0l, RT_Negative); return B; };
	inline XInt fma(const XInt A, const XInt B, const XInt C){ return (A * B) + C; };
	inline XInt fmax(const XInt A, const XInt B){ if (A >= B) return A; return B; };
	inline XInt fmin(const XInt A, const XInt B){ if (A < B) return A; return B; };
	inline XInt fmod(const XInt A, const XInt B){ return A % B; };
	inline XInt ldexp(const XInt A, int B){ XInt Two(2l); return A * Two.Pow(XInt(B)); };
	inline XInt log(const XInt A){ return A.Log(XInt(XI_E)); };
	inline XInt log10(const XInt A){ return A.Log(XInt(10l)); };
	inline XInt log1p(const XInt A){ return log(A + XInt(1l)); };
	inline XInt log2(const XInt A){ return A.Log(XInt(2l)); };
	inline XInt logb(const XInt A){ return A.Abs().Log(XInt(2l)); };
	inline XInt lrint(const XInt A){ XInt B(A); B.Round(0); B.SetDecimals(0); return B; };
	inline XInt llround(const XInt A){ return lrint(A); };
	inline XInt modf(const XInt A, XInt* B){ *B = A.FracPart(); return A.IntPart(); };
	inline XInt nearbyint(const XInt A){ return lrint(A); };
	inline XInt pow(const XInt A, const XInt B){ return A.Pow(B); };
	inline XInt remainder(const XInt A, const XInt B){ return A % B; };
	inline XInt remquo(const XInt A, const XInt B, XInt* C){ *C = A / B;  C->Round(0); C->SetDecimals(0); return A % B; };
	inline XInt rint(const XInt A){ XInt B(A);  B.Round(0); B.SetDecimals(0); return B; };
	inline XInt round(const XInt A){ XInt B(A); B.Round(0l, RT_Nearest, RT5_AwayFromZero); return B; };
	inline XInt sin(const XInt A){ return A.Sin(10); };
	inline XInt sqrt(const XInt A){ return A.Root(XInt(2l)); };
	inline XInt tan(const XInt A){ return A.Tan(10); };
	inline XInt trunc(const XInt A){ return A.IntPart(); };

	//Streaming - not declared as friends because they only use public interfaces

	std::ostream& operator<<(std::ostream& os, const XInt& A);
	std::istream& operator>>(std::istream& os, XInt& A);

} //namespace

