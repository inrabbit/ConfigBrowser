#ifndef DEF_OWNED_UNIV_TYPE
#define DEF_OWNED_UNIV_TYPE

#include "UnivType.h"

class Equation;

// ���L�ҏ��t����UnivType�A�����̎Z�o���ʂł���s��̊e�v�f�Ƃ��ė��p����

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
