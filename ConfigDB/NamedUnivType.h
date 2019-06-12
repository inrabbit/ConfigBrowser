#ifndef DEF_NAMED_UNIV_TYPE
#define DEF_NAMED_UNIV_TYPE

#include "UnivType.h"

#define MAX_NAME_LENGTH 32

class NamedUnivType : public UnivType
{
private:
	char *m_pName;

public:
	NamedUnivType();
	virtual ~NamedUnivType();

	void setName(const char *pName);
	const char *getName() const { return m_pName; }

};

#endif
