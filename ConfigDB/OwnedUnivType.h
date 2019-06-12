#ifndef DEF_OWNED_UNIV_TYPE
#define DEF_OWNED_UNIV_TYPE

#include "UnivType.h"

class Equation;

// 所有者情報付きのUnivType、数式の算出結果である行列の各要素として利用する

class OwnedUnivType : public UnivType
{
private:
	Equation *m_pOwner;

public:
	OwnedUnivType(){ m_pOwner = NULL; }
	virtual ~OwnedUnivType(){ UnivType::~UnivType(); }

	void setOwner(Equation *pOwner);
	Equation *getOwner() const { return m_pOwner; }
	void modified();
};

#endif
