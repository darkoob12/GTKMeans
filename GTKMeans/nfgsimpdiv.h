#ifndef NFGSIMPDIV_H
#define NFGSIMPDIV_H
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "libgambit/libgambit.h"


class nfgSimpdiv {
private:
	int m_nRestarts, m_leashLength;

	int t, ibar;
	Gambit::Rational d,pay,maxz,bestz;

	Gambit::Rational Simplex(Gambit::MixedStrategyProfile<Gambit::Rational> &);
	Gambit::Rational getlabel(Gambit::MixedStrategyProfile<Gambit::Rational> &yy, Gambit::Array<int> &, Gambit::PVector<Gambit::Rational> &);
	void update(Gambit::RectArray<int> &, Gambit::RectArray<int> &, Gambit::PVector<Gambit::Rational> &,
		const Gambit::PVector<int> &, int j, int i);
	void getY(Gambit::MixedStrategyProfile<Gambit::Rational> &x, Gambit::PVector<Gambit::Rational> &, 
		const Gambit::PVector<int> &, const Gambit::PVector<int> &, 
		const Gambit::PVector<Gambit::Rational> &, const Gambit::RectArray<int> &, int k);
	void getnexty(Gambit::MixedStrategyProfile<Gambit::Rational> &x, const Gambit::RectArray<int> &,
		const Gambit::PVector<int> &, int i);
	int get_c(int j, int h, int nstrats, const Gambit::PVector<int> &);
	int get_b(int j, int h, int nstrats, const Gambit::PVector<int> &);

public:
	nfgSimpdiv(void);
	virtual ~nfgSimpdiv();

	int NumRestarts(void) const { return m_nRestarts; }
	void SetNumRestarts(int p_nRestarts) { m_nRestarts = p_nRestarts; }

	int LeashLength(void) const { return m_leashLength; }
	void SetLeashLength(int p_leashLength) { m_leashLength = p_leashLength; }

	void Solve(const Gambit::Game &, const Gambit::MixedStrategyProfile<Gambit::Rational> &);
};

void PrintProfile(std::ostream &p_stream,
	const std::string &p_label,
	const Gambit::MixedStrategyProfile<Gambit::Rational> &p_profile);

bool ReadProfile(std::istream &p_stream, Gambit::MixedStrategyProfile<Gambit::Rational> &p_profile);

Gambit::Integer find_lcd(const Gambit::Vector<Gambit::Rational> &vec);
void Randomize(Gambit::MixedStrategyProfile<Gambit::Rational> &p_profile, int p_denom);
void PrintBanner(std::ostream &p_stream);
void PrintHelp(char *progname);

#endif