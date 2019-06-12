#ifndef DEF_REQUIRE_UPDATE_OBJ
#define DEF_REQUIRE_UPDATE_OBJ

#define UK_FLAG_VALUE       0x01	// 値の更新
#define UK_FLAG_ARRAY_SIZE  0x02	// 配列のサイズの更新

enum UpdateKind{
	UK_ONLY_VALUE       = UK_FLAG_VALUE,	// 値のみの更新
	UK_ONLY_ARRAY_SIZE  = UK_FLAG_ARRAY_SIZE,	// 配列のサイズのみの更新
	UK_ALL              = UK_FLAG_VALUE | UK_FLAG_ARRAY_SIZE	// 両者を伴う
};

class UnivType;
class BaseDB;

class RequireUpdateObj
{
private:
	bool m_updated;
	static BaseDB *m_pDB;

protected:
	// オブジェクトの更新状態を設定
	void invalidate(){ m_updated = false; }
	void validate(){ m_updated = true; }
	// オブジェクトに更新が必要か判定する
	bool requireUpdate(){ return !m_updated; }

public:
	RequireUpdateObj();
	~RequireUpdateObj();

	static void setDB(BaseDB *pDB){ m_pDB = pDB; }
	static BaseDB *getDB(){ return m_pDB; }

	virtual bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	virtual void update();

};

#endif
