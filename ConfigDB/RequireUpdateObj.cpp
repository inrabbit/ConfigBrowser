#include "RequireUpdateObj.h"
#include "BaseDB.h"

BaseDB *RequireUpdateObj::m_pDB = NULL;

RequireUpdateObj::RequireUpdateObj()
{
	// 更新リストに追加
	m_pDB->addToUpdateList(this);
	// 更新が必要
	invalidate();
}

RequireUpdateObj::~RequireUpdateObj()
{
	// 更新リストから削除
	m_pDB->removeFromUpdateList(this);
}

bool RequireUpdateObj::receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind)
{
	return false;
}

void RequireUpdateObj::update()
{
	// デフォルトは何もしない
}
