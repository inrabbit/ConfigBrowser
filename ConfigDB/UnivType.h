#ifndef DEF_UNIV_TYPE
#define DEF_UNIV_TYPE

#include "CdbException.h"
#include "CommonUtils.h"

//#define UNION_AS_STRUCT	// for debug

class UnivMatrix;
class IndexConv;
class Equation;
class BaseDB;
enum UpdateKind;

class UnivType
{
public:
	// �����^��`
	typedef float Real;
	typedef double Double;

protected:
	// �^�C�v���ʎq
	enum ContentKind{
		TYPE_EMPTY = 0, 
		TYPE_REF, 
		TYPE_REF_CONST, 
		TYPE_MATRIX, 
		TYPE_EQUATION, 
		TYPE_INT, 
		TYPE_UINT, 
		TYPE_REAL, 
		TYPE_DOUBLE, 
		TYPE_POINTER, 
		TYPE_BOOL, 
		TYPE_STRING, 
		TYPE_BINARY, 
		TYPE_SUBSET, 
		TYPE_INVALID
	};

	// ���ێ����p��
#ifdef UNION_AS_STRUCT
	struct CommonType
#else
	union CommonType
#endif
	{
		int integer;
		unsigned int uinteger;
		void *ptr;
		bool boolean;
		Real real;
		Double dreal;
		Equation *pEquation;
		UnivMatrix *pMatrix;
		struct tagRefer{
			UnivType *pRefer;
			IndexConv *pIndexConv;
		}refer;
		struct tagString{
			char *pString;
			size_t length;
		}string;
		struct tagBinData{
			unsigned char *pData;
			size_t size;
		}binData;
		struct tagSubset{
			BaseDB *pDB;
		}subset;
	};

	// �^���E�����ێ�
#if 1
	struct TypeInfo{
		unsigned int hasName       :	1;
		unsigned int hasOwner      :    1;
		unsigned int isConstant    :	1;
		unsigned int isObjectOwner :    1;
		unsigned int contentKind   :	4;
	};
#else
	struct TypeInfo{
		unsigned char hasName;			// ���O�t�����ǂ���(NamedUnivType�ɃL���X�g�\)
		unsigned char hasOwner;			// ���L����Ă��邩(OwnedUnivType�ɃL���X�g�\)
		unsigned char isConstant;		// �萔�������ǂ���
		unsigned char isObjectOwner;	// �q�I�u�W�F�N�g�����g�̎������ł��邩(true�Ȃ�Ώ��Ŏ��ɊJ������)
		unsigned char contentKind;		// �^�C�v���ʎq
	};
#endif

protected:
	// �f�[�^�ێ��̋��p��
#ifdef UNION_AS_STRUCT
	struct CommonType m_data;
#else
	union CommonType m_data;
#endif

	// �f�[�^�̎�ʂƃf�[�^�T�C�Y
	struct TypeInfo m_type;
	static struct TypeInfo m_initial_type;

	// �T�u�Z�b�g�̐ݒ�
	//////////////////////////////////////////////////////////////////////////////
	// �y���ӁzBaseDB�݂̂��T�u�Z�b�g���쐬�ł���B
	// ���[�U�[��BaseDB::createSubset()�𗘗p���邱�ƁB
	//////////////////////////////////////////////////////////////////////////////
	// BaseDB�݂̂��T�u�Z�b�g���쐬�ł��邽�߁A�t�����h�w�肵�Ă���
	friend BaseDB;
public:
	// �n���ꂽBaseDB�I�u�W�F�N�g�����b�v����subset�I�u�W�F�N�g�𐶐�����
	// delegated == true �ɐݒ肷��ƁA�ȍ~�̑����Ǘ���UnivType�Ɉڂ�
	void setSubset(BaseDB *pDB, bool delegated = true);

private:
	// �����̕]���i��O�ɕϐ�����t�����ē����Ȃ����j
	inline UnivType& evalEquation() const;
	// �ϐ�����t�����ė�O�𓊂���i�z��v�f�ԍ��͕t������Ȃ��j
	void throwException(CDB_ERR eErrorCode) const;
	// �Q�ƃI�u�W�F�N�g�̎擾�i�C���f�b�N�X�ϊ��̌��ʁA�v�f�����P�ɂȂ����s����X�J���[�Ƃ��ĕԂ��j
	// �C���f�b�N�X�ϊ������݂���ꍇ�ŁA�ϊ��̌��ʂɗv�f�����P�łȂ��ꍇ�͗�O�ƂȂ�
	inline UnivType& getScalarReference();
	inline const UnivType& getScalarReference() const;
	// �Q�ƃI�u�W�F�N�g�̎擾�i�C���f�b�N�X�ϊ��̌��ʁA�v�f�����P�ɂȂ����s����X�J���[�Ƃ��ĕԂ��j
	// �C���f�b�N�X�ϊ��̌��ʂɗv�f�����P�łȂ��ꍇ�͎Q�ƃI�u�W�F�N�g���̂��̂�Ԃ�
	inline const UnivType& getReferenceForTypeInspection() const;
	//////////////////////////////////////////////////////////////////////////////
	// �y���ӁzUnivType�ł́A�P�~�P�s��ƃX�J���[��ʕ��Ƃ��Ĉ����B
	// �������A�C���f�b�N�X�ϊ��̌��ʂP�~�P�ƂȂ�Q�ƍs��̏ꍇ�A���@��̓��ʋK���ɂ���ăX�J���[�Ƃ��Ĉ����B
	//////////////////////////////////////////////////////////////////////////////
	// ���p���O�łȂ��ꍇ�A�Q�Ƃɂ����ăC���f�b�N�X�ϊ��𔺂����ǂ������擾����
	inline bool isIndexConvAvailable() const;
	// ���p���O�ɁA�Q�Ƃɂ����ăC���f�b�N�X�ϊ��𔺂����ǂ������擾����(�����ɃC���f�b�N�X�ϊ��̍X�V���s��)
	inline bool isIndexConvExists() const;

public:
	UnivType(){ Initialize(); }
	UnivType(int x){ Initialize(); setInt(x); }
	UnivType(unsigned int x){ Initialize(); setUInt(x); }
	UnivType(Real x){ Initialize(); setReal(x); }
	UnivType(Double x){ Initialize(); setDouble(x); }
	UnivType(const char *p){ Initialize(); setString(p); }
	UnivType(bool x){ Initialize(); setBool(x); }
	UnivType(UnivType& src) { Initialize(); move(src); }
	//UnivType(const UnivType& src) { Initialize(); copy(src); }
	virtual ~UnivType(){ clearAll(); }

	void Initialize();
	void clearAll();	// �t���Q�Ƃ��܂߂������N���A
	void clear();		// �ʏ�̃N���A
	void invalidate();
	void copy(const UnivType& src);		// ���e�̑S�R�s�[	
	void evalCopy(const UnivType& src);	// ������]�����Ă��猋�ʂ��R�s�[
	void move(UnivType& src);

	// �X�V�ʒm�̎󂯎��
	bool receiveNotifyUpdated(const UnivType *pUnivType, enum UpdateKind kind);
	void update();

	// �Q�Ɛ�̒u������
	bool replaceReference(UnivType *pFindWhat, UnivType *pReplaceWith);

	void setMatrix(int row, int col);
	// ��const�I�u�W�F�N�g�ɑ΂����const�s��v�f�̎擾
	UnivType& operator[](int n){ return getAt(n); }
	UnivType& getAt(int n);
	UnivType& getAt(int row, int col);
	// const�I�u�W�F�N�g�ɑ΂���const�s��v�f�̎擾
	const UnivType& operator[](int n) const { return getAtConst(n); }
	const UnivType& getAt(int n) const { return getAtConst(n); }
	const UnivType& getAt(int row, int col) const { return getAtConst(row, col); }
	// ��const�I�u�W�F�N�g�ɑ΂��閾���I��const�s��v�f�̎擾
	const UnivType& getAtConst(int n) const;
	const UnivType& getAtConst(int row, int col) const;

	int getNumRow() const;
	int getNumColumn() const;
	int getNumElements() const;

	int getInt() const;
	unsigned int getUInt() const;
	void *getPointer() const;
	bool getBool() const;
	Real getReal() const;
	Double getDouble() const;
	const char *getString() const;
	size_t getStringLength() const;
	void *getBinary() const;
	size_t getBinarySize() const;
	BaseDB *getSubsetDB() const;

	void toString(UnivType& obj) const;
	void toBinary(UnivType& obj) const;
	void toSubsetDB(UnivType& obj) const;

	char *getFormatString(char *pBuff, size_t sizeBuff) const;
#ifdef DEBUG_PRINT
	void debugPrintStructure(int nIndentLevel, bool collapsed = true) const;
#endif

	void setInt(int x);
	void setUInt(unsigned int x);
	void setPointer(void *p);
	void setBool(bool x);
	void setReal(Real x);
	void setDouble(Double x);
	void setString(const char *p);
	void setBinary(const void *p, size_t size);

	// IndexConv�I�u�W�F�N�g��ݒ肷��ƁA�ȍ~�̑����Ǘ���UnivType�Ɉڂ�(�Q�Ɛ�͊Ǘ����Ȃ�)
	//////////////////////////////////////////////////////////////////////////////
	// �y���Ӂz�Q�Ɛݒ�́A���̐ݒ�ɗD�悳���B
	// �Q�Ɛݒ肳��Ă���UnivType��setInt()�ȂǂŐݒ���s���Ă��A�Q�Ɛ�ɐݒ肳���݂̂ł���B
	// �Q�Ɛݒ����������ɂ́A�Q�Ɛ��NULL���w�肵�ĉ������邵���Ȃ��B
	//////////////////////////////////////////////////////////////////////////////
	void setFullReference(UnivType *pRefer, IndexConv *pIndexConv = NULL);
	void setConstReference(const UnivType *pRefer, IndexConv *pIndexConv = NULL);
	void getReference(UnivType*& pRefer, IndexConv*& pIndexConv) const;
	void getReference(const UnivType*& pRefer, IndexConv*& pIndexConv) const;

	// �Q�Ƃ̉���(�t���Q�Ƃ����������i�͂��̊֐����ĂԂ����Ȃ�)
	void releaseReference();

	// �Q�Ƃ��l�X�g����Ă���ۂɁA�Q�Ƃ���Ă���Ő�̃I�u�W�F�N�g�𓾂�
	// isGoBackOnlyThroughRef = { true:�X���[�Q�Ƃ݂̂�k��, false:�C���f�b�N�X�ϊ��t�������l�ɑk�� }
	const UnivType *getReferenceRoot(bool isGoBackOnlyThroughRef) const;

	// Equation�I�u�W�F�N�g��ݒ肷��ƁA�ȍ~�̑����Ǘ���UnivType�Ɉڂ�
	void setEquation(Equation *pEquation);
	Equation *getEquation();
	const Equation *getConstEquation() const;

	operator int() const { return getInt(); }
	operator unsigned int() const { return getUInt(); }
	operator short() const { return static_cast<short>(getInt()); }
	operator unsigned short() const { return static_cast<unsigned short>(getUInt()); }
	operator void *() const { return getPointer(); }
	operator bool() const { return getBool(); }
	operator Real() const { return getReal(); }
	operator Double() const { return getDouble(); }
	operator const char *() const { return getString(); }

	UnivType& operator=(int x){ setInt(x); return *this; }
	UnivType& operator=(unsigned int x){ setUInt(x); return *this; }
	UnivType& operator=(Real x){ setReal(x); return *this; }
	UnivType& operator=(Double x){ setDouble(x); return *this; }
	UnivType& operator=(const char *p){ setString(p); return *this; }
	UnivType& operator=(bool x){ setBool(x); return *this; }
	UnivType& operator=(const UnivType& x){ copy(x); return *this; }

	int getContentTypeID() const;

	// �萔�̏ꍇ�̉��Z�̍�����
	void simplify() const;

	// �萔����(�A�N�Z�X����ۂɁA�������݂������邩�ǂ���)
	bool isConstant() const;
	void setConstant(bool isConstant);

	// fixed����(�ˑ��ϐ��̒l�̕ω��ɉe�������\���������true��Ԃ�)
	bool isFixed() const;

	// �Q�Ƃł����true��Ԃ�
	bool isReference() const { return (m_type.contentKind == TYPE_REF) || (m_type.contentKind == TYPE_REF_CONST); }

	// �t���Q�Ƃł����true��Ԃ�
	bool isFullReference() const { return m_type.contentKind == TYPE_REF; }

	// const(�����݋֎~��)�Q�Ƃł����true��Ԃ�
	bool isConstReference() const { return m_type.contentKind == TYPE_REF_CONST; }

	// ���̃I�u�W�F�N�g���L���ł����true��Ԃ�
	bool isValid() const;

	// �����ێ����Ă��Ȃ����true��Ԃ�
	bool isEmpty() const;

	// ���ł����true��Ԃ�
	bool isEquation() const;

	// �X�J���[�l�i�z��łȂ��j�Ȃ��true��Ԃ�
	bool isScalar() const;

	// �s��i�܂��̓x�N�g���j�Ȃ��true��Ԃ�
	bool isMatrix() const;

	// �����ł����true��Ԃ�
	bool isInteger() const;
	
	// �����Ȃ������ł����true��Ԃ�
	bool isUInteger() const;
	
	// �����ł����true��Ԃ�
	bool isReal() const;
	bool isDouble() const;

	// �|�C���^�ł����true��Ԃ�
	bool isPointer() const;

	// BOOL�^�ł����true��Ԃ�
	bool isBoolean() const;

	// ������ł����true��Ԃ�
	bool isString() const;

	// �o�C�i���I�u�W�F�N�g�ł����true��Ԃ�
	bool isBinary() const;

	// �T�u�Z�b�g�ł����true��Ԃ�
	bool isSubset() const;

	// ���l�ł����true��Ԃ�
	bool isNumber() const { return isInteger() || isUInteger() || isReal() || isDouble(); }

	// �����Ȃ����l�ł����true��Ԃ�
	bool isUnsignedNumber() const { return isUInteger(); }

};

#endif
