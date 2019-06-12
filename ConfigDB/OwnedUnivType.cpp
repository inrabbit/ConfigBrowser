#include "OwnedUnivType.h"
#include "Equation.h"

void OwnedUnivType::setOwner(Equation *pOwner)
{
	m_pOwner = pOwner;
	if(pOwner == NULL){
		m_type.hasOwner = FALSE;
	}else{
		m_type.hasOwner = TRUE;
	}
}

void OwnedUnivType::modified()
{
	m_pOwner->fixCache();
	m_pOwner->clearWriteEventHook();
}
