#ifndef DEF_PARSER
#define DEF_PARSER

class UnivType;
class TokenAnaly;
class BaseDB;
class IndexConv;

enum TokenKind;
class SubsetContext;
class BraceContext;
class SentenceContext;
class IndexContext;
class CatContext;

class Parser
{
private:
	TokenAnaly& m_token;				// �g�[�N���擾�I�u�W�F�N�g
	BaseDB& m_RootDB;					// ��͓��e�̓o�^��DB
	BaseDB *m_pCustomFuncArguments;		// ���[�U�[��`�֐��̈������o�^DB�i�g��Ȃ��ꍇ��NULL�j

	// �g�[�N������֐�
	enum TokenKind getNextToken();				// ���̃g�[�N���̎�ʂ𓾂�
	bool checkNextToken(enum TokenKind kind);	// ���̃g�[�N���̎�ʂ𔻒肷��i�s��v�Ȃ�P�߂��j
	bool assertNextToken(enum TokenKind kind);	// ���̃g�[�N���̎�ʂ𔻒肷��i�s��v�Ȃ�G���[�j
	void getCurrentTokenContent(UnivType& obj);	// ���݂̃g�[�N���̓��e���擾����
	void putBackToken();						// �擾�����g�[�N����߂�

	// �I�u�W�F�N�g�擾�֐�
	UnivType& getObjectByName(const char *pName);

private:
	// ���݂̃R���e�L�X�g�ێ��ϐ�
	SubsetContext *m_pSubsetContext;
	BraceContext *m_pBraceContext;
	SentenceContext *m_pSentenceContext;
	IndexContext *m_pIndexContext;
	CatContext *m_pCatContext;

private:
	// �\����͊֐�
	void topScript();					// �X�N���v�g�\����̓g�b�v
	void subsetDefine();				// �T�u�Z�b�g(�u���b�N)��`(�O���[�o���ʒu�A���O������)
	void topEquation();					// ����̓g�b�v
	UnivType& topLeft();				// ���ӎ��\����̓g�b�v
	UnivType& topFunc();				// �֐���`���i���Ӂj�\����̓g�b�v
	void topRight(UnivType& obj);		// �E�ӎ��g�b�v

	// �E�ӎ���͊֐�
	void branch(UnivType& obj);			// ����
	void condition(UnivType& obj);		// ������
	void matrix(UnivType& obj);			// �s��
	void vector(UnivType& obj);			// �x�N�g��
	void expression(UnivType& obj);		// ���i�{,�|�Ȃǂɂ�錋���j
	void term(UnivType& obj);			// ���i��,���Ȃǂɂ�錋���j
	void unaryOp(UnivType& obj);		// �P�����Z�i�]�u�A�]�u�����Ȃǁj
	void subsetAccess(UnivType& obj);	// �T�u�Z�b�g�A�N�Z�X���Z
	bool subsetDeclare(UnivType& obj);	// �T�u�Z�b�g��`(�E�ӎ��A���O�����)
	bool func(UnivType& obj);			// �֐��E�z��A�N�Z�X���Z
	bool primitive(UnivType& obj);		// ��{�v�f

	void expression2nd(UnivType& obj, UnivType& first);	// ���i�Q�v�f�ڈȍ~�j
	void term2nd(UnivType& obj, UnivType& first);		// ���i�Q�v�f�ڈȍ~�j

	// �E�ӎ���͊֐�
	void matrixLeft(UnivType& obj);		// �s��
	void vectorLeft(UnivType& obj);		// �x�N�g��
	void primitiveLeft(UnivType& obj);	// ��{�v�f

	// �z��C���f�b�N�X��͊֐�
	void arrayIndexRight(IndexConv *pIndexConv, const UnivType *pArray, int nDim);	// �z��C���f�b�N�X�i�E�ӎ��j
	void arrayIndexLeft(IndexConv *pIndexConv, const UnivType *pArray, int nDim);	// �z��C���f�b�N�X�i���ӎ��j

public:
	Parser(TokenAnaly& token, BaseDB& database);
	~Parser();

	void execute();	// ��͂̎��s
	void interpret(UnivType& obj);	// �E�ӎ��̉�͂̎��s
	bool generateReference(UnivType& ref);	// ���̂��߂ɂ���̂��s��

};

#endif
