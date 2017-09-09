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

//For non Windows/MFC builds, you will need to comment 
//out the next line and replace it with:
//#include "XInt.h"

#include "stdafx.h"
#include <sstream>
#include <iostream>
#include <iomanip>

namespace TRS{

XInt::XInt(XI_IX Val, XI_IX Exponent){

	m_pWords = NULL;
	m_NumWords = 0;
	m_AllocatedWords = 0;
	GrowTo(1);
	m_pWords[0] = ::abs(Val);
	if (Val < 0)
		m_Negative = true;
	else
		m_Negative = false;
	m_Infinite = false;
	m_NaN = false;
	m_Exponent = Exponent;

	Rationalise();

}

XInt::XInt(const XInt* pOther){

	m_pWords = NULL;
	m_NumWords = 0;
	m_AllocatedWords = 0;
	m_Negative = pOther->m_Negative;

	if (pOther->IsInfinite()){

		MakeInfinite();
		return;

	}

	if (pOther->IsNaN()){

		MakeNaN();
		return;

	}

	GrowTo(pOther->m_NumWords);
	memcpy(m_pWords, pOther->m_pWords, pOther->m_NumWords * sizeof(XI_WORD));
	m_Infinite = false;
	m_NaN = false;
	m_Exponent = pOther->m_Exponent;

}

XInt::XInt(const std::string str){

	m_pWords = NULL;
	m_NumWords = 0;
	m_AllocatedWords = 0;

	*this = str;

}

XInt::~XInt(){

	delete m_pWords;

}

void XInt::MakeZero(){

	GrowTo(1);
	m_pWords[0] = 0;
	m_Negative = false;
	m_Infinite = false;
	m_NaN = false;
	m_Exponent = 0;

}

void XInt::MakeInfinite(){

	delete m_pWords;
	m_pWords = NULL;
	m_NumWords = 0;
	m_Infinite = true;
	m_NaN = false;
	m_Exponent = 0;

}

void XInt::MakeNaN(){

	delete m_pWords;
	m_pWords = NULL;
	m_NumWords = 0;
	m_NaN = true;
	m_Negative = false;
	m_Infinite = false;
	m_Exponent = 0;

}

void XInt::RationaliseWords(){

	//Each XI_WORD is only used to hold up to XI_MAX minus 1
	//which leaves us a few bits spare for temporary over/underflows.
	//This procedure carries the excess to the next XI_WORD, 
	//allocating more as necessary.

	for (XI_IX i = 0; i < m_NumWords; i++){

		if (m_pWords[i] >= XI_MAX){

			if (i == m_NumWords - 1)
				GrowBy(1);

			XI_WORD Carry = m_pWords[i] / XI_MAX;

			m_pWords[i + 1] += Carry; //If this causes another overflow we catch it on the next loop through
			m_pWords[i] -= (Carry * XI_MAX);

		}

	}

}

void XInt::Rationalise(){

	XI_IX	i;

	if (!IsNumber())
		return;

	RationaliseWords();

	//We do not allow positive exponents to be passed outside
	//of the class. First we insert as many whole words as we can.

	if (m_Exponent >= XI_DECS_PER_WORD){

		XI_IX WordsToInsert = m_Exponent / XI_DECS_PER_WORD;

		GrowBy(WordsToInsert);

		for (XI_IX i = 0; i < WordsToInsert; ++i){

			m_pWords[i + WordsToInsert] = m_pWords[i];
			m_pWords[i] = 0;

		}

		m_Exponent -= (WordsToInsert * XI_DECS_PER_WORD);

	}

	//Then we insert any part word that is necessary.

	while (m_Exponent > 0){

		for (i = 0; i < m_NumWords; i++)
			m_pWords[i] *= 10;

		--m_Exponent;

		RationaliseWords();

	}

	//Clip off any leading zeroes. There's no need to reallocate the memory,
	//we just pretend it isn't there and it will be released whenever the 
	//rest of the memory is.

	while (m_NumWords > 1 && m_pWords[m_NumWords - 1] == 0)
		m_NumWords--;

	//A value of zero is always positive

	if (IsZero())
		m_Negative = false;

}

void XInt::GrowBy(XI_IX NumWords){

	if (NumWords <= 0){

		//We don't actually delete the memory,
		//we just pretend it has gone. 

		m_NumWords += NumWords;
		return;

	}

	//NumWords > 0
	//Is there already enough space allocated?

	XI_IX NewWords = m_NumWords + NumWords;

	if (NewWords > m_AllocatedWords){

		XI_IX Alloc = NewWords + XI_EXTRA_WORDS;
		XI_WORD* pNew = new XI_WORD[Alloc];

		if (m_pWords){

			memcpy(pNew, m_pWords, m_NumWords * sizeof(XI_WORD));
			delete m_pWords;

		}

		m_pWords = pNew;
		m_AllocatedWords = Alloc;

	}

	for (XI_IX i = m_NumWords; i < NewWords; ++i)
		m_pWords[i] = 0;

	m_NumWords = NewWords;

}

void XInt::SetDecimals(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5){

	while (m_Exponent < -NumDecimals){

		Div10(RT,R5);
		++m_Exponent;

	}

	Rationalise();

	while (m_Exponent > -NumDecimals){

		for (XI_IX i = 0; i < m_NumWords; ++i)
			m_pWords[i] *= 10;

		--m_Exponent;
		Rationalise();

	}

}

void XInt::Round(XI_IX Position, XI_RoundingType RT, XI_RoundingFive R5){

	//This procedure modifies the decimal digit at Position depending on 
	//the rounding type and (possibly) the value of the digit at Position - 1.
	//It then zeroes all the digits after Position. Position[0] is the first
	//Digit to the left of the decimal point, negative values are decimals.
	
	XI_IX RealPosition = Position - m_Exponent;
	
	//If RealPosition is less than zero, it's in the implied zeros at
	//the end and 0 does not round in any rounding model. Similarly, if it
	//is the last position (RealPosition = 0) we have nothing to round with.
	//IOW, in either case, the number is already rounded.

	if (RealPosition <= 0)
		return;

	//If RealPosition is off to the left of the significant digits, then we
	//can just set ourselves to zero and stop. The special case is if RealPosition
	//is only one greater, In this case, it could potentially round up. E.G. if
	//this is equal to 99 and we are rounding to Position 2 it could round to 100 
	//depending on the rounding model. 

	if (RealPosition > NumSignificantDigits()){

		MakeZero();
		return;

	}

	bool	NeedRounding = false;
	XI_IX	Digit = (*this)[Position];
	XI_IX	RoundingDigit = (*this)[Position - 1];

	if (RoundingDigit != 0){  //Zero never rounds

		switch (RT) {

			case RT_Truncate:
			case RT_TowardsZero:
				break; //Truncation has no effect

			case RT_Positive:
				if (!m_Negative)
					NeedRounding = true;
				break;

			case RT_Negative:
				if (m_Negative)
					NeedRounding = true;
				break;

			case RT_AwayFromZero:
				NeedRounding = true;
				break;

			case RT_Nearest:

				if (RoundingDigit > 5)
					NeedRounding = true;

				if (RoundingDigit == 5){

					bool	NextDigitEven;
					XI_IX	Digit = (*this)[Position];

					if ((Digit % 2) != 0)
						NextDigitEven = true;
					else
						NextDigitEven = false;

					switch (R5){

						case RT5_Positive:
							if (!m_Negative)
								NeedRounding = true;
							break;

						case RT5_Negative:
							if (m_Negative)
								NeedRounding = true;
							break;

						case RT5_TowardsZero:
							break;

						case RT5_AwayFromZero:
							NeedRounding = true;
							break;

						case RT5_Even:
							if (!NextDigitEven)
								NeedRounding = true;
							break;

						case RT5_Odd:
							if (NextDigitEven)
								NeedRounding = true;
							break;

				}

			}

		}

	}

	//When we get to here, we have to round one digit up if NeedRounding is true.

	if (NeedRounding){

		//We do that by figuring out which word the digit is in and it's relative 
		//position in that word. We then add one to that position in that word.

		XI_WORD* pW = m_pWords + (RealPosition / XI_DECS_PER_WORD);
		XI_WORD Mask = 1;
		XI_IX Offset = RealPosition % XI_DECS_PER_WORD;

		while (Offset > 0){

			--Offset;
			Mask *= 10;

		}

		*pW += Mask;

		//Rounding could have created a non-rational value - i.e. a
		//word that is greater than XI_MAX 

		Rationalise();

	}

	//Finally, we need to truncate

	XI_WORD* pW = m_pWords;
	int i;

	//First, zero out any complete words

	while (RealPosition > XI_DECS_PER_WORD){

		RealPosition -= XI_DECS_PER_WORD;
		*pW = 0;
		++pW;

	}

	//Now zero out the trailing digits of the significant word

	for (i = 0; i < RealPosition; ++i)
		*pW /= 10;

	for (i = 0; i < RealPosition; ++i)
		*pW *= 10;

	//It could have gone all the way to zero, in which case we have to fix the sign.

	if (IsZero())
		m_Negative = false;

}

void XInt::Unpack(XI_WORD* pA, XI_IX NumHalfWords) const{

	XI_WORD	Carry;
	XI_IX	i;

	for (i = 0; i < m_NumWords; ++i){

		Carry = m_pWords[i] / XI_HALF_MAX;
		pA[(i * 2) + 1] = Carry;
		pA[i * 2] = m_pWords[i] - (Carry * XI_HALF_MAX);

	}

	for (i = m_NumWords * 2; i < NumHalfWords; i++)
		pA[i] = 0;

}

void XInt::Pack(XI_WORD* pA, XI_IX NumHalfWords){

	XI_IX	i;

	GrowTo(NumHalfWords / 2);

	for (i = 0; i < m_NumWords; i++)
		m_pWords[i] = pA[i * 2] + (pA[(i * 2) + 1] * XI_HALF_MAX);

	Rationalise(); 

}

void XInt::AddWords(const XInt& Other){

	//Number of words and exponent are already equal

	for (int i = 0; i < m_NumWords; ++i)

		//We can allow overflow as each XI_WORD is only used to hold 
		//up to XI_MAX and two of those don't cause an actual 
		//overflow. Rationalise() will sort it all out.

		m_pWords[i] += Other.m_pWords[i];

	Rationalise();

}

void XInt::SubtractWords(const XInt& Other){

	XInt	B(Other);

	//Number of words and exponent are already equal

	for (XI_IX i = 0; i < m_NumWords; i++){

		//We can't allow underflow as each XI_WORD is unsigned 
		//so we use the "borrow one" technique you should have
		//learned in primary school! We already know that *this
		//is larger than Other so it can't happen at the highest 
		//Word.

		if (m_pWords[i] < B.m_pWords[i]){

			m_pWords[i] += XI_MAX;
			B.m_pWords[i+1]++;

		}

		m_pWords[i] -= B.m_pWords[i];

	}

	Rationalise();

}

void XInt::Align(XInt& A, XInt& B){

	//Here, Big and Small refer to the exponent.
	//The highest exponent has the smallest number
	//of decimal places so we have to reduce the
	//exponent by increasing the main number.
	//Multiplying the main number by 10 reduces
	//the exponent by one. 

	XInt*	pSmall;
	XInt*	pBig;

	if (A.m_Exponent != B.m_Exponent){

		if (A.m_Exponent < B.m_Exponent){

			pSmall = &A;
			pBig = &B;

		}

		else{

			pSmall = &B;
			pBig = &A;

		}

		//See if we can insert whole words, which is much faster

		if (pBig->m_Exponent - pSmall->m_Exponent >= XI_DECS_PER_WORD){

			XI_IX WordsToInsert = (pBig->m_Exponent - pSmall->m_Exponent) / XI_DECS_PER_WORD;
			XI_IX NewWords = WordsToInsert + pBig->m_NumWords;

			//If we don't have enough memory allocated, allocate some more

			pBig->GrowTo(NewWords);

			//Now we can left-shift into the allocated space, 
			//filling with zeros from the right

			//Left shift
			for (XI_IX i = NewWords - 1; i >= WordsToInsert; --i)
				pBig->m_pWords[i] = pBig->m_pWords[i - WordsToInsert];

			//Zero fill
			for (XI_IX i = 0; i < WordsToInsert; ++i)
				pBig->m_pWords[i] = 0;

			pBig->m_Exponent -= (WordsToInsert * XI_DECS_PER_WORD);

		}

		//Single digits

		while (pBig->m_Exponent > pSmall->m_Exponent){

			for (XI_IX i = 0; i < pBig->m_NumWords; ++i)
				pBig->m_pWords[i] *= 10;

			--pBig->m_Exponent;

			pBig->RationaliseWords();

		}

	}

	//Zero-pad to make the two numbers the same number of words

	if (A.m_NumWords > B.m_NumWords)
		B.GrowTo(A.m_NumWords);

	if (B.m_NumWords > A.m_NumWords)
		A.GrowTo(B.m_NumWords);

}

XInt XInt::Add(const XInt& Other) const{

	//If either number is infinite, the result will be infinite

	if (m_Infinite || Other.m_Infinite){

		XInt Result;

		//unless they both are and their signs are opposite in which 
		//case the result is 0

		if (m_Infinite && Other.m_Infinite && m_Negative != Other.m_Negative)
			return Result;

		Result.MakeInfinite();

		//The sign of the one that is infinite dominates

		if (m_Infinite)
			Result.m_Negative = m_Negative;
		else
			Result.m_Negative = Other.m_Negative;

		return Result;

	}

	//If either number is not a number, the result will be not a number

	if (m_NaN || Other.m_NaN){

		XInt Result;
		Result.MakeNaN();

		return Result;

	}

	XInt	A(this), B(Other), Result;
	bool	ABiggest;

	A.m_Negative = false;
	B.m_Negative = false;

	//Before we can add or subtract the numbers, they have to be aligned
	//at the decimal point. 

	Align(A,B);

	//If both numbers have the same sign, we can just add the Words

	if (Other.m_Negative == m_Negative){

		Result = A;
		Result.AddWords(B);
		Result.m_Negative = m_Negative;

		return Result;

	}

	//But if they are different, we subtract the Words
	//Subtraction isn't commutative though so it depends
	//on which one is negative
	//a + (-b) = a - b
	//-a + b = b - a
	//We also need the absolute value of the first one 
	//to be greater than the second or we get an underflow
	//on the highest order (last) Word which screws 
	//everything up.
	//So we solve it all by subtracting the smallest absolute
	//From the biggest and then sort the sign out later.

	if (A > B)
		ABiggest = true;
	else
		ABiggest = false;

	if (ABiggest){

		A.SubtractWords(B);
		Result = A;

	}

	else{ 

		B.SubtractWords(A);
		Result = B;

	}

	//Whichever one was biggest, its sign dominates

	if (!Result.IsZero())
		if (ABiggest)
			Result.m_Negative = m_Negative;
		else
			Result.m_Negative = Other.m_Negative;

	return Result;

}

XInt XInt::Multiply(const XInt& Other) const{

	XInt		Result;
	XI_IX		NumHalfWords, i, Row, Col;
	XI_WORD*	pA;
	XI_WORD*	pB;
	XI_WORD*	aResult;
	XI_WORD*	aWork;
	XI_WORD		Carry;

	//If either number is zero, the result is zero and we don't need to
	//do the multiplication.

	if (IsZero() || Other.IsZero())
		return Result;

	//If both numbers have the same sign, the result is +ve, else -ve

	if (Other.m_Negative != m_Negative)
		Result.m_Negative = true;

	//If either number is infinite, the result will be infinite

	if (m_Infinite || Other.m_Infinite){

		Result.MakeInfinite();
		return Result;

	}

	//If either number is NaN, the result will be NaN

	if (m_NaN || Other.m_NaN){

		Result.MakeNaN();
		return Result;

	}

	//We break out the Words of both numbers into half the base size.
	//This gives us enough room to perform intermediate multiplications 
	//without any chance of overflow.
	//The max. possible number of half words in the result is the sum of
	//the number of Words in the 2 numbers being multiplied times 2 
	//for the half base size.

	NumHalfWords = (m_NumWords + Other.m_NumWords) * 2;

	pA = new XI_WORD[NumHalfWords];
	pB = new XI_WORD[NumHalfWords];
	aResult = new XI_WORD[NumHalfWords];
	aWork = new XI_WORD[NumHalfWords];
	
	Unpack(pA, NumHalfWords);
	Other.Unpack(pB, NumHalfWords);
	memset(aResult, 0, NumHalfWords * sizeof(XI_WORD));
	memset(aWork, 0, NumHalfWords * sizeof(XI_WORD));

	//Now we do a big matrix multiplication summing the result
	//into the total after each line. Row is the row number but
	//also the offset for the result

	for (Row = 0; Row < Other.m_NumWords * 2; Row++){

		for (Col = 0; Col < m_NumWords * 2; Col++)
			aWork[Col + Row] = pA[Col] * pB[Row];

		//Sum this row into the result

		XI_WORD* pResult = aResult;
		XI_WORD* pWork = aWork;

		for (i = 0; i < NumHalfWords; ++i,++pResult,++pWork)
			*pResult += *pWork; 

		//Rationalise aResult back to half words
		//and clean out aWork for another pass

		for (i = 0; i < NumHalfWords; ++i){

			if (aResult[i] >= XI_HALF_MAX){

				Carry = aResult[i] / XI_HALF_MAX; 
				aResult[i + 1] += Carry; //If this causes another overflow we catch it on the next loop through
				aResult[i] = aResult[i] - (Carry * XI_HALF_MAX);

			}

			aWork[i] = 0;

		}

	}

	Result.Pack(aResult, NumHalfWords); 
	Result.m_Exponent = m_Exponent + Other.m_Exponent;

	Result.Rationalise();

	delete pA;
	delete pB;
	delete aResult;
	delete aWork;

	return Result;

}

XInt XInt::Divide(const XInt& Other, XI_IX Dec, XI_RoundingType RT, XI_RoundingFive R5) const{

	//Handle all the special cases first

	XInt	Result;

	//If either number is zero, the result is zero or infinity or NaN and 
	//we don't need to do the division.

	if (Other.IsZero() && IsZero()){

		Result.MakeNaN();
		return Result;

	}

	if (IsZero())
		return Result;  //Zero

	if (Other.IsZero()){

		Result.MakeInfinite();
		Result.m_Negative = m_Negative;
		return Result;

	}

	//If either number is NaN, the result will be NaN

	if (m_NaN || Other.m_NaN){

		Result.MakeNaN();
		return Result;

	}

	//If either number is infinite, the result will be +/- infinity or one

	if (m_Infinite){

		if (Other.m_Infinite)
			Result = (XI_IX)1;
		else
			Result.MakeInfinite();

		if (m_Negative == Other.m_Negative)
			Result.m_Negative = false;
		else
			Result.m_Negative = true;

		return Result;

	}

	if (Other.m_Infinite)
		return Result;

	//If they have the same numerical value, the answer is +/- 1

	XInt One(1);
	XInt A = Abs();
	XInt B = Other.Abs();

	if (A == B){

		if (m_Negative != Other.m_Negative)
			One.m_Negative = true;

		return One;

	}

	//If the bottom is +/- one, return +/- the top

	if (B == One){

		Result = A;

		if (m_Negative != Other.m_Negative)
			Result.m_Negative = true;

		return Result;

	}

	//When we get here, A and B are positive.

	ResolveDP(Other, Dec);

	//We do a subtract and shift type of division

	XInt	PartResult;
	XI_IX	Shift = 0;

	//First we have to shift B until it is just less than A.
	//The shift can be in either direction.

	while (B < A){

		++B.m_Exponent;
		//B.Rationalise();
		++Shift;

	}

	while (A < B){

		--B.m_Exponent;
		//B.Rationalise();
		--Shift;

	}

	//Now we subtract, shift and repeat

	while (!A.IsZero() && Shift >= -Dec - 1){

		PartResult.MakeZero();

		while (A >= B){

			A -= B;
			++PartResult;

		}

		PartResult.m_Exponent = Shift;
		Result += PartResult;
		--Shift;
		--B.m_Exponent;

	}

	if (m_Negative != Other.m_Negative)
		Result.m_Negative = true;

	Result.SetDecimals(Dec, RT, R5);

	return Result;

}

void XInt::ResolveDP(const XInt& Other, XI_IX& Dec) const{

	switch (Dec){

		case DP_LHS:

			Dec = -m_Exponent;
			break;

		case DP_RHS:

			Dec = -Other.m_Exponent;
			break;

		case DP_SUM:

			Dec = -abs(Other.m_Exponent + m_Exponent);
			break;

		case DP_MUL:

			Dec = -abs(Other.m_Exponent * m_Exponent);
			break;

		default:

			if (Dec < 0)
				Dec = 0;
			break;

	}

}

void XInt::ResolveDP(XI_IX Dec) const{

	switch (Dec){

		case DP_LHS:

			Dec = -m_Exponent;
			break;

		default:

			if (Dec < 0)
				Dec = 0;
			break;

	}

}

void XInt::Div2(){

	//This is a stripped down version of the regular division
	//code optimised for dividing by 2. In fact, we multiply 
	//by 5 and decrement the exponent.

	for (XI_IX i = 0; i < m_NumWords; ++i)
		m_pWords[i] *= 5;

	--m_Exponent;

	Rationalise();

}

void XInt::Div10(XI_RoundingType RT, XI_RoundingFive R5){

	//This is a stripped down version of division
	//optimised for dividing by 10.

	XI_WORD	Carry = 0, NewCarry, CarryVal = XI_MAX / 10;
	XI_IX	SaveExp;

	SaveExp = m_Exponent;
	m_Exponent = 0;
	Round(1, RT, R5);
	m_Exponent = SaveExp;

	for (XI_IX i = m_NumWords - 1; i >= 0; i--){

		NewCarry = (m_pWords[i] % 10) * CarryVal;
		m_pWords[i] /= 10;
		m_pWords[i] += Carry;
		Carry = NewCarry;

	}

}

XInt XInt::Root(const XInt n) const{

	XInt	Zero,One(1l),Log2,Result,Two(2l);

	if (n.IsZero())  
		return One;

	if (IsZero())
		return Zero;

	if (m_Negative){

		Result.MakeNaN();
		return Result;

	}

	if (n.m_Negative)
		return Pow(-n);

	Log2 = Log(Two,30);
	Log2 = Log2.Divide(n, 30);
	Result = Log2.AntiLog(Two);

	return Result;

}

XInt XInt::operator %(const XInt N) const{

	XInt Result, Div, M;
	
	if (N.IsZero()){

		Result = *this;
		return Result;

	}

	M = N.IntPart();

	Div = Divide(M, 0, RT_Truncate);
	Result = IntPart() - (M * Div);

	return Result;

}

XInt XInt::Exponentiate(const XInt& n) const{

	//First we check the terminating conditions.

	if (n.IsZero())
		return XInt(1); //Anything to the power of 0 is 1

	if (n == XInt(1))
		return *this; //Anything to the power of 1 is itself

	if (n == XInt(2))
		return *this * *this; //Anything to the power of 2 is itself times itself

	//A fast way to calculate exponents is to make
	//use of the fact that x^n = x^(n/2) * x^(n/2)
	//if n is even and x^n = x^(n/2) * x^(n/2) * x
	//if n is odd.

	//Because we are dealing with integers, we may not
	//be able to halve them exactly. The following logic
	//sets l to n/2 truncated and m to n/2 (plus one where
	//l got truncated). If n is odd, l and m will not be the 
	//same i.e. if we are calculating X^5, it
	//will calculate x^2 * x^2 * x.

	XInt l = n.Divide(XInt(2),0,RT_Truncate);
	XInt m = n - l;
	XInt lExp = Exponentiate(l); //Recursion

	XInt Result = lExp * lExp;

	if (l != m)
		Result *= *this;

	return Result;

}

XInt XInt::Pow (const XInt n) const{

	//A negative exponent is actually one over the positive exponent. 
	//i.e. x to the power of -n is one over x to the n.

	if (n.m_Negative){

		XInt One(1),Exp;

		Exp =  Pow(-n);

		return One.Divide(Exp, DP_RHS);

	}

	//We now handle fractional exponents. To do that,
	//we use logs. 

	if (!n.FracPart().IsZero()){

		XInt	Log2;

		Log2 = Log();
		Log2 *= n;

		return Log2.AntiLog();

	}

	return Exponentiate(n);

}

XI_WORD XInt::operator [](const XI_IX Index) const{

	XInt		A(this);
	XI_IX		RealIndex = Index - m_Exponent;
	XI_WORD		Res;
	XI_WORD*	pW = A.m_pWords;

	if (RealIndex < 0 || RealIndex >= NumSignificantDigits())
		return 0; //Index out of range so it must be an implied zero

	//First skip over any whole words

	while (RealIndex >= XI_DECS_PER_WORD){

		RealIndex -= XI_DECS_PER_WORD;
		++pW;

	}

	//Now pW points to the word and RealIndex is the offset in that 
	//word. Shuffle the word over until the digit we want is the lowest
	//in that word

	while (RealIndex > 0){

		*pW /= 10;
		--RealIndex;

	}

	//Now ignore all digits except the last one

	Res = *pW % 10;
	
	return Res;

}

XI_IX XInt::NumSignificantDigits() const{

	//All of the words except the highest are fully used
	XI_IX Result;
	
	Result = (m_NumWords - 1) * XI_DECS_PER_WORD;

	//Count the used decimals in the high word
	XI_WORD High = m_pWords[m_NumWords - 1];

	while (High){

		++Result;
		High /= 10;

	}

	if (m_Exponent > 0)
		Result += m_Exponent;

	return Result;

}

XI_Comp XInt::Compare(const XInt& Other) const{

	if (!IsNumber() || !Other.IsNumber()){

		//For non-numeric (INF or NaN), we return:
		//
		//  other->  +INF  -INF  NaN  Number
		//  this
		//  +INF      EQ    GT    GT    GT
		//  -INF      LT    EQ    LT    LT
		//  NaN       LT    GT    EQ    LT
		//  Number    LT    GT    GT    XX
		//
		//+INF is greater than everything except +INF
		//-INF is less than everything except -INF
		//NaN is the problem child. The above 2 rules 
		//cover 2 of the 4 cases. NaN compares equal
		//to NaN which covers the 3rd case. For the
		//final case, we arbitarily treat NaN as less 
		//than any real number.
		//The case labelled XX (both numbers) can't 
		//happen inside this block as we already know
		//that at least one side is non-numeric. That
		//case is dealt with below.

		if (m_Infinite){ 

			if (!m_Negative){ //First row of the table above
			
				if (Other.m_Infinite && !Other.m_Negative)
					return Compare_EQ;

				return Compare_GT;
			
			}

			else{ //Second row of the table above
			
				if (Other.m_Infinite && Other.m_Negative)
					return Compare_EQ;

				return Compare_LT;
			
			}

		}

		if (m_NaN){ //Third row of the table above
		
			if (Other.m_Infinite){

				if (Other.m_Negative)
					return Compare_GT;

				return Compare_LT;

			}

			if (Other.m_NaN)
				return Compare_EQ;

			return Compare_LT;

		}

		//Fourth row of the table above
	
		if (Other.m_Infinite && !Other.m_Negative)
			return Compare_LT;

		return Compare_GT;

	}

	//If we get here, both are valid numbers

	XInt A(this), B(Other);

	Align(A, B);

	for (XI_IX i = A.m_NumWords - 1; i >= 0; --i){

		if (A.m_pWords[i] > B.m_pWords[i])
			return Compare_GT;

		if (A.m_pWords[i] < B.m_pWords[i])
			return Compare_LT;

	}

	return Compare_EQ;

}

XInt& XInt::operator = (const XInt& Other){

	m_Negative = Other.m_Negative;
	m_Exponent = Other.m_Exponent;

	if (Other.m_Infinite){
	
		MakeInfinite();
		return *this;

	}

	if (Other.m_NaN){
	
		MakeNaN();
		return *this;

	}

	GrowTo(Other.m_NumWords);
	memcpy(m_pWords, Other.m_pWords, m_NumWords * sizeof(XI_WORD));
	m_Infinite = false;
	m_NaN = false;

	return *this;

}

#ifdef _AFX

void XInt::operator = (const CString& Source){

	std::string		s;
	CStringA		Work;

	Work = Source;
	s = Work.GetString();

	*this = s;

}

XInt::operator CString () const{

	std::string	s(*this);
	CString		Ret;
	
	Ret = s.c_str();

	return Ret;

}

#endif

void XInt::operator = (const std::string& Source){

	XI_IX	SaveExp=0;
	bool	SaveNeg=false;

	if (Source.empty()){

		*this = (XI_IX)0;
		return;

	}

	std::string	Work = Source;
	XI_IX		Last;
	char*		ws = " \t\n\r\f\v";

	//Make the whole string upper case which
	//simplifies processing of NAN, INF and 
	//hex digits.

	for (size_t i = 0; i < Work.length(); i++)
		Work[i] = toupper(Work[i]);

	//Remove leading and trailing whitespace 

	Work.erase(0, Work.find_first_not_of(ws)); //ltrim
	Work.erase(Work.find_last_not_of(ws) + 1); //rtrim

	//We allow one leading or trailing sign 
	//with or without embedded whitespace

	m_Negative = false;

	if (Work[0] == '-'){

		SaveNeg = true;
		Work.erase(0,1);
		Work.erase(0, Work.find_first_not_of(ws)); //ltrim

	}

	else
		if (Work[0] == '+'){

			Work.erase(0,1);
			Work.erase(0, Work.find_first_not_of(ws)); //ltrim

		}

	Last = Work.length() - 1;

	if (Work[Last] == '-'){

		SaveNeg = true;
		Work.erase(Last);
		Work.erase(Work.find_last_not_of(ws) + 1); //rtrim

	}

	else
		if (Work[Last] == '+'){

			Work.erase(Last);
			Work.erase(Work.find_last_not_of(ws) + 1); //rtrim

		}

	if (Work == "INF"){

		MakeInfinite();
		m_Negative = SaveNeg;
		return;

	}

	if (Work == "NAN"){

		MakeNaN();
		return;

	}

	MakeZero();

	if (Work == "0" || Work == "0.0" || Work == "0X0")
		return;

	//Find out if there is a decimal point. If so, store
	//its position and delete it from the string.

	size_t DecPos = Work.find('.');

	if (DecPos != std::string::npos){

		SaveExp = DecPos - Work.length() + 1;
		Work.erase(DecPos, 1);

		while (Work[0] == '0')
			Work.erase(0, 1);

	}

	//Determine the base

	XI_IX Base=10;

	if (Work[0] == '0'){

		//Hex always starts with 0X

		if (Work[1] == 'X'){

			Base = 16;
			Work.erase(0, 2);

		}

		//Octal starts with 0 and contains no digit greater than 7
		
		else if (Work.find_first_not_of("01234567") == std::string::npos){

			Base = 8;
			Work.erase(0, 1);

		}

	}

	//The position in this table is the numerical value
	char	Trans[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	XI_IX	Digit;

	for (std::string::size_type i = 0; i < Work.length(); ++i){

		for (Digit = 0; Digit < Base && Work[i] != Trans[Digit]; ++Digit);

		if (Digit == Base)
			Digit = 0; //Treat any invalid character as 0

		*this *= Base;
		*this += Digit;

	}

	m_Exponent = SaveExp;
	m_Negative = SaveNeg;

}

XInt::operator std::string () const{

	std::stringstream	str_str;
	std::string			str;

	str_str << *this;
	str = str_str.str();

	return str;

}

void XInt::operator = (const long double Source){

	char Buffer[1000];
	std::string Str;

	if (isnan(Source)){

		MakeNaN();
		return;

	}

	if (isinf(Source)){

		MakeInfinite();
		m_Negative = isless(Source, 0.0);
		return;

	}

	sprintf_s(Buffer, 1000, "%.30f", Source);
	Str = Buffer;
	*this = Str;

}

XInt::operator long double() const{

	long double	LD;
	XI_IX		i;

	if (m_NaN)
		return std::numeric_limits<double>::quiet_NaN();

	if (m_Infinite){

		LD = std::numeric_limits<double>::infinity();

		if (m_Negative)
			LD *= -1.0;

		return LD;

	}

	//The next line throws up a "possible loss of data"
	//warning in some builds (mainly 64 bit builds). In
	//practice it can't happen as we don't use the entire
	//word and its maximum value (XI_MAX - 1) isn't enough 
	//to cause an overflow. For builds bigger than 64 bit 
	//this will be a problem. Hopefully, by then there will
	//be a bigger float type we can use.

	LD = m_pWords[m_NumWords - 1];

	for (i = m_NumWords - 2; i >= 0; --i){

		LD *= XI_MAX;
		LD += m_pWords[i];

	}

	for (i = 0; i > m_Exponent; --i)
		LD /= 10.0;

	for (i = 0; i < m_Exponent; ++i)
		LD *= 10.0;

	if (m_Negative)
		LD *= -1.0;

	return LD;

}


XInt::operator XI_IX() const{

	if (*this < XInt(-XI_IX_MAX))
		return -XI_IX_MAX;

	if (*this > XInt(XI_IX_MAX))
		return XI_IX_MAX;

	XInt	Work = IntPart();
	XI_IX	Ret = Work.m_pWords[Work.m_NumWords - 1];

	for (XI_IX i = Work.m_NumWords - 2; i >= 0; --i){

		Ret *= XI_MAX;
		Ret += Work.m_pWords[i];

	}

	if (m_Negative)
		Ret = -Ret;

	return Ret;

}


void XInt::operator = (const XI_IX Source){

	m_Infinite = false;
	m_NaN = false;
	GrowTo(1);
	m_Exponent = 0;
	
	if (Source < 0){

		m_Negative = true;
		m_pWords[0] = -Source;

	}
	
	else{

		m_Negative = false;
		m_pWords[0] = Source;

	}

}

bool XInt::IsZero() const{

	if (!IsNumber())
		return false;
	
	for (int i = 0; i < m_NumWords; ++i)
		if (m_pWords[i] != 0)
			return false;

	return true;

}

XInt XInt::Abs() const{

	XInt Result(this);
	Result.m_Negative = false;
	
	return Result;

}

XInt XInt::Log(const XInt& Base,XI_IX NumDecimals) const{

	XInt Result,Work(this),Bits(1l),One(1l),Two(2l);
	
	if (IsNumber() && IsNegative()){

		Result.MakeNaN();
		return Result;

	}

	if (IsNaN() || IsInfinite())
		return *this;

	if (IsZero()){

		Result.MakeInfinite();
		return Result;

	}

	Bits.Div2();

	while (Work < One){

		Work *= Two;
		--Result;

	}

	while (Work >= Two){

		Work.Div2();
		++Result;

	}

	ResolveDP(NumDecimals);

	for (XI_IX i = 0; i <= (NumDecimals * 4); ++i){

		Work *= Work;
		Work.SetDecimals(NumDecimals);
		Work.Rationalise();

		if (Work >= Two){

			Work.Div2();
			Result += Bits;

		}

		Bits.Div2();
	
	}

	Result.Rationalise();
	Result.SetDecimals(NumDecimals,RT_Nearest,RT5_AwayFromZero);

	if (Base != Two)
		Result = Result.Divide(Base.Log(Two,NumDecimals),NumDecimals); 

	return Result;

}

XInt XInt::AntiLog(const XInt& Base, XI_IX NumDecimals) const{

	XInt	Result, Two(2l), FPart, IPart, Partial, Work(this);
	XI_IX	Digit,NumDigits,WorkDecimals;

	ResolveDP(NumDecimals);
	WorkDecimals = NumDecimals + 4;

	if (Base != Two)
		Work = Work.Divide(Base.Log(Two, NumDecimals), NumDecimals);

	IPart = Work.IntPart();
	FPart = Work.FracPart();

	if (FPart.m_Negative){

		--IPart;
		++FPart;

	}

	Two.SetDecimals(NumDecimals);
	Result = Two.Pow(IPart);

	if (!FPart.IsZero()){

		//This array contains the pre-calculated values for 2 
		//to the power of 0.1, 0.01, etc. to 31 decimal places.
		//These are the best values I can find, if you have access
		//to better ones or know where I can find them, please 
		//let me know.

		std::string	s[] = {
			"not used",
			"1.0717734625362931642130063250233", //.1
			"1.0069555500567188088326982141132", //.01
			"1.0006933874625806325375686393039", //.001
			"1.0000693171203765691924399126026", //.0001
			"1.0000069314958283056532090898006", //.00001
			"1.0000006931474207865077726362274", //.000001
			"1.0000000693147204582596560368400", //.0000001
			"1.0000000069314718296221038455865", //.00000001
			"1.0000000006931471808001718164318", //.000000001
			"1.0000000000693147180583967960114", //.0000000001
			"1.0000000000069314718056234757449", //.00000000001
			"1.0000000000006931471805601855359", //.000000000001
			"1.0000000000000693147180559969332", //.0000000000001
			"1.0000000000000069314718055994771", //.00000000000001
			"1.0000000000000006931471805599455", //.000000000000001
			"1.0000000000000000693147180559945", //.0000000000000001
			"1.0000000000000000069314718055995", //.00000000000000001
			"1.0000000000000000006931471805599", //.000000000000000001
			"1.0000000000000000000693147180560", //.0000000000000000001
			"1.0000000000000000000069314718056", //.00000000000000000001
			"1.0000000000000000000006931471806", //.000000000000000000001
			"1.0000000000000000000000693147181", //.0000000000000000000001
			"1.0000000000000000000000069314718", //.00000000000000000000001
			"1.0000000000000000000000006931472", //.000000000000000000000001
			"1.0000000000000000000000000693147", //.0000000000000000000000001
			"1.0000000000000000000000000069315", //.00000000000000000000000001
			"1.0000000000000000000000000006931", //.000000000000000000000000001
			"1.0000000000000000000000000000693", //.0000000000000000000000000001
			"1.0000000000000000000000000000069", //.00000000000000000000000000001
			"1.0000000000000000000000000000007", //.000000000000000000000000000001
			"1.0000000000000000000000000000001"  //.0000000000000000000000000000001
		};

		XI_IX ns = sizeof(s) / sizeof(std::string);

		NumDigits = FPart.NumSignificantDigits();

		for (int i = 1; i < NumDigits && i < ns; ++i){

			Digit = FPart[-i];

			if (Digit){

				Partial = s[i];

				for (XI_IX j = 0; j < Digit; ++j)
					Result *= Partial;

				Result.SetDecimals(WorkDecimals);
				Result.Rationalise();

			}

		}

	}

	Result.SetDecimals(NumDecimals);
	Result.Rationalise();

	return Result;

}

XInt XInt::IntPart() const{

	//Returns everything to the left of the decimal point

	XInt Result(this);

	Result.SetDecimals(0, RT_Truncate);

	return Result;

}

XInt XInt::FracPart() const{

	//Returns everything to the right of the decimal point

	XInt Result(this);
	Result -= IntPart();

	return Result;

}

XInt XInt::operator ~() const{

	XInt Result(this);

	for (int i = 0; i < Result.m_NumWords; ++i){

		Result.m_pWords[i] = ~Result.m_pWords[i];
		Result.m_pWords[i] %= XI_MAX;

	}

	return Result;

}

XInt XInt::operator |(const XInt& Other) const{

	XInt A(this), B(Other);

	Align(A, B);

	for (int i = 0; i < A.m_NumWords; ++i)
		A.m_pWords[i] |= B.m_pWords[i];

	A.Rationalise();

	return A;

}

XInt XInt::operator &(const XInt& Other) const{

	XInt A(this), B(Other);

	Align(A, B);

	for (int i = 0; i < A.m_NumWords; ++i)
		A.m_pWords[i] &= B.m_pWords[i];

	A.Rationalise();

	return A;

}

XInt XInt::operator ^(const XInt& Other) const{

	XInt A(this), B(Other);

	Align(A, B);

	for (int i = 0; i < A.m_NumWords; ++i)
		A.m_pWords[i] ^= B.m_pWords[i];

	A.Rationalise();

	return A;

}

XInt XInt::operator >>(const XI_IX Places) const{

	XInt	Result(this), Two(2l);

	Two = Two.Pow(XInt(Places));
	Result = Result.Divide(Two, DP_LHS);

	return Result;

}

XInt XInt::operator <<(const XI_IX Places) const{

	XInt	Result(this), Two(2l);

	Two = Two.Pow(XInt(Places));
	Result *= Two;

	return Result;

}

void XInt::Base10ToString(std::string& str) const{

	//We can do base 10 a lot faster by streaming it out a word at a time

	XI_IX					i,InsertPoint;
	std::ostringstream		ss;

	ss << m_pWords[m_NumWords - 1];

	for (i = m_NumWords - 2; i >= 0; --i)
		ss << std::setw(XI_DECS_PER_WORD) << std::setfill('0') << m_pWords[i];

	str = ss.str();

	//Insert decimal point

	InsertPoint = str.length() + m_Exponent;

	//If the insert point is less than zero, insert some leading zeros

	if (InsertPoint <= 0){

		while (InsertPoint < 1){

			str.insert(0, 1, '0');
			++InsertPoint;

		}

	}

	//Insert the decimal point if it isn't the last character

	if (InsertPoint < (int)str.length())
		str.insert(InsertPoint, 1, '.');

	//Insert the sign

	if (m_Negative)
		str.insert(0, 1, '-');

}

void XInt::NumToString(std::string& str, XI_IX Base) const{

	char	Digit[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	XInt	B(Base), Work(this), Bottom(Base), Zero, One(1l);
	int		Count;
	bool	DidDP=false;

	if (IsZero()){

		str += '0';
		return;

	}

	Work.m_Negative = false;
	
	while (Bottom <= Work)
		Bottom *= B;

	while (Bottom > Work)
		Bottom = Bottom.Divide(B, -Bottom.m_Exponent + 1);

	if (Bottom < One){

		str += "0.";
		DidDP = true;

		for (XI_IX i = -1; i > Bottom.m_Exponent; --i)
			str += '0';

	}

	while (Work > Zero || Bottom >= One){

		Count = 0;

		while (Work >= Bottom){

			Work -= Bottom;
			Count++;

		}

		str += Digit[Count];
		Bottom = Bottom.Divide(B, -Bottom.m_Exponent + 1);

		if (!DidDP && Bottom < One){

			str += '.';
			DidDP = true;

		}

	}

	if (str[str.length() - 1] == '.')
		str.erase(str.length() - 1);

}

void XInt::StreamOut(std::ostream& os) const{

	//This is a basic implementation and will be completed later to
	//take into account other ostream characteristics (precision etc.) 

	XI_IX		Base;
	std::string	str;

	if (m_Infinite){

		if (m_Negative)
			str = '-';

		str += "INF";
		os << str;

		return;

	}

	if (m_NaN){

		os << "NaN";

		return;

	}

	if (m_Negative)
		str = '-';

	std::ios_base::fmtflags base = os.flags() & os.basefield;

	switch (base){
	
		case std::ios::hex:

			Base = 16;
			str += "0x";
			break;

		case std::ios::oct:

			Base = 8;
			str += "0";
			break;

		default:

			Base = 10;

	}

	//Fractions are only allowed for decimal output

	if (Base != 10 && !FracPart().IsZero()){

		os << "NaN";

		return;

	}

	if (Base == 10)
		Base10ToString(str);
	else
		NumToString(str, Base);

	os << str;

}

void XInt::StreamIn(std::istream& is){

	std::string	str;

	is >> str;
	*this = str;

}

std::ostream& operator<<(std::ostream& os, const XInt& A){
	
	A.StreamOut(os);
	return os;

}

std::istream& operator>>(std::istream& is, XInt& A){
	
	A.StreamIn(is);
	return is;

}

XInt XInt::Sin(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	ResolveDP(NumDecimals);

	XInt	Work(this);
	XInt	Result;
	XInt	TwoPi(XI_TWO_PI);

	while (Work > TwoPi)
		Work -= TwoPi;

	while (Work.m_Negative)
		Work += TwoPi;

	if (Work.IsZero())
		return Result;

	//We use the Taylor expansion for sin(x):

	//x ** 1 / 1! -
	//x ** 3 / 3! +
	//x ** 5 / 5! -
	//x ** 7 / 7! + 
	//...

	//Note that we do not have to calculate the powers and
	//factorials from scratch each time as we can use the
	//previous value as a starting point.
	//x ** 5 = x ** 3 * x * x and
	//5! = 3! * 4 * 5

	//We stop when the next term is less than the acceptible 
	//error.

	XInt	MaxError(1l);

	MaxError.m_Exponent = -NumDecimals - 3;

	XInt	Top(Work);
	XInt	Bottom(1l);
	XInt	Partial(Work);
	XInt	Ix(1l);
	XInt	WorkSquared;

	WorkSquared = Work * Work;

	do{

		Result += Partial;

		Top.Flip(); //This gives us the alternating sign
		Top *= WorkSquared;

		++Ix;
		Bottom *= Ix;
		++Ix;
		Bottom *= Ix;

		Partial = Top.Divide(Bottom, NumDecimals + 1, RT, R5);

		Top.Flip(); //This gives us the alternating sign
		Top *= WorkSquared;

		++Ix;
		Bottom *= Ix;
		++Ix;
		Bottom *= Ix;

		Partial += Top.Divide(Bottom, NumDecimals + 1, RT, R5);

	} while (Partial.Abs() > MaxError);

	Result.SetDecimals(NumDecimals, RT, R5);

	return Result;

}

XInt XInt::Cos(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	if (IsZero())
		return XInt(1l);

	//The cosine of any angle is the same as the sine of 90 degrees - the angle

	XInt	Work;
	XInt	HalfPi(XI_HALF_PI);

	Work = HalfPi - *this;

	return Work.Sin(NumDecimals, RT, R5);

}

XInt XInt::Tan(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	//Tan(x) is Sin(x) / Cos(x)

	return Sin(NumDecimals + 2, RT, R5).Divide(Cos(NumDecimals + 2, RT, R5), NumDecimals, RT, R5);

}

XInt XInt::ArcTan(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	//Our DoArcTan function gets slower and slower as x gets larger.
	//From empirical tests, the performance is acceptable when x is
	//less than 0.7 (approx).
	//We can use the three following identities to ensure that x is 
	//always within the range 0 < x < 0.7:

	//arctan (-x) = -arctan(x)
	//arctan(x) = arctan(c) + arctan((x - c) / (1 + x*c))
	//arctan(1 / x) = 0.5 * pi - arctan(x)[x > 0]

	XInt	Work(this);
	XInt	Point7("0.7");

	//arctan (-x) = -arctan(x)
	//This gets everything less than 0

	if (Work.IsNegative()){

		Work.m_Negative = false;
		Work = XInt(XI_TWO_PI) - Work.ArcTan(NumDecimals, RT, R5);
		Work.SetDecimals(NumDecimals, RT, R5);

		return Work;

	}

	//If we get here, x >= 0

	if (Work.IsZero()){

		Work.MakeZero();
		Work.SetDecimals(NumDecimals);
		
		return Work;

	}

	//If we get here, x > 0
	//If it's less than 0.7, we can calculate it directly

	if (Work < Point7)
		return DoArcTan(NumDecimals, RT, R5);

	//If we get here, x >= 0.7
	//For the range 0.7 <= x < 1, we have to use the 2nd identity
	//arctan(x) = arctan(c) + arctan((x - c) / (1 + x*c))

	XInt One(1l);

	if (Work <= One){

		XInt C = Work.Divide(XInt(2l), NumDecimals, RT, R5);
		XInt D = (Work - C).Divide(One + (Work * C), NumDecimals, RT, R5);

		return C.DoArcTan(NumDecimals, RT, R5) + D.ArcTan(NumDecimals, RT, R5);

	}

	//If we get here, x > 1.0
	//For the range 1.0 < x < Infinity, we have to use the 3rd identity
	//arctan(1 / x) = 0.5 * pi - arctan(x)[x > 0]
	//but it helps to reorder it:
	//arctan(x) = (0.5 * pi) - arctan(1 / x)

	if (!Work.IsInfinite()){

		Work = XInt(XI_HALF_PI) - One.Divide(Work,NumDecimals, RT, R5).ArcTan(NumDecimals, RT, R5);
		Work.SetDecimals(NumDecimals, RT, R5);

		return Work;

	}

	//If we get here, x = Infinity

	if (Work.m_Negative)
		Work = XInt(XI_HALF_PI) + XInt(XI_PI);
	else
		Work = XInt(XI_HALF_PI);

	Work.SetDecimals(NumDecimals, RT, R5);

	return Work;

}

XInt XInt::DoArcTan(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	ResolveDP(NumDecimals);

	XInt	Work(this);
	XInt	Result;

	//We use the Taylor expansion for arctan(x):

	//x ** 1 / 1 -
	//x ** 3 / 3 +
	//x ** 5 / 5 -
	//x ** 7 / 7 + 
	//...

	//Note that we do not have to calculate the powers 
	//from scratch each time as we can use the
	//previous value as a starting point.
	//x ** 5 = x ** 3 * x * x and
	//5! = 3! * 4 * 5

	//We stop when the next term is less than the acceptible 
	//error.

	XInt	MaxError(1l);

	MaxError.m_Exponent = -NumDecimals - 3;

	XInt	Top(Work);
	XInt	Bottom(1l);
	XInt	Partial(Work);
	XInt	Two(2l);
	XInt	WorkSquared;

	Bottom.SetDecimals(NumDecimals + 1);
	Two.SetDecimals(NumDecimals + 1);

	WorkSquared = Work * Work;

	do{

		Result += Partial;

		Top.Flip(); //This gives us the alternating sign
		Top *= WorkSquared;
		Bottom += Two;
		Partial = Top.Divide(Bottom, NumDecimals + 1, RT, R5);

		Top.Flip(); //This gives us the alternating sign
		Top *= WorkSquared;
		Bottom += Two;
		Partial += Top.Divide(Bottom, NumDecimals + 1, RT, R5);

	} while (Partial.Abs() > MaxError);

	Result.SetDecimals(NumDecimals, RT, R5);

	return Result;

}

XInt XInt::ArcSin(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	XInt	Y;
	XInt	Two(2l);
	XInt	One(1l);

	if (Abs() > One){

		Y.MakeNaN();
		return Y;

	}

	//ArcSin(x) = ArcTan(x/sqrt(1 - x^2))

	ResolveDP(NumDecimals);

	Y = One - (*this * *this);
	Y = Y.Root(Two);
	Y = this->Divide(Y, NumDecimals, RT, R5);

	return Y.ArcTan(NumDecimals, RT, R5);

}

XInt XInt::ArcCos(XI_IX NumDecimals, XI_RoundingType RT, XI_RoundingFive R5) const{

	XInt	Y;
	XInt	Two(2l);
	XInt	One(1l);

	if (Abs() > One){

		Y.MakeNaN();
		return Y;

	}

	//ArcCos(x) = ArcTan(sqrt(1 - x^2)/x)

	ResolveDP(NumDecimals);

	Y = One - (*this * *this);
	Y = Y.Root(Two);
	Y = Y.Divide(*this, NumDecimals, RT, R5);

	return Y.ArcTan(NumDecimals, RT, R5);

}

} // namespace
