#ifndef DEF_UPDATE_MANAGER
#define DEF_UPDATE_MANAGER

#if 0	// refÇ≈ÇÕëΩèdåpè≥Ç™ñ≥óùÇÁÇµÇ¢
ref class UpdateListener abstract
{
public:
	UpdateListener(){};
	~UpdateListener(){};
	virtual void NotifyUpdated() abstract;
};
#endif

#include <vector>

#define WT_FORM_UNIVTYPE_LIST 5963
#define WT_FORM_MATRIX_VIEW   5964
#define WT_INVALID            6000

ref class UpdateManager
{
	static array<System::Windows::Forms::Form^>^ m_pUpdateListenerList;
	static int m_count;
public:

	UpdateManager();
	static void AddToUpdateList(int type, System::Windows::Forms::Form^ pObj);
	static void RemoveFromUpdateList(System::Windows::Forms::Form^ pObj);
	static void NotifyUpdate();
};

#endif
