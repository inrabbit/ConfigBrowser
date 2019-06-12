#include "RequireUpdateObj.h"
#include "BaseDB.h"

BaseDB *RequireUpdateObj::m_pDB = NULL;

RequireUpdateObj::RequireUpdateObj()
{
	// �X�V���X�g�ɒǉ�
	m_pDB->addToUpdateList(this);
	// �X�V���K�v
	invalidate();
}

RequireUpdateObj::~RequireUpdateObj()
{
	// �X�V���X�g����폜
	m_pDB->removeFromUpdateList(this);
}

bool RequireUpdateObj::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	return false;
}

void RequireUpdateObj::update()
{
	// �f�t�H���g�͉������Ȃ�
}
