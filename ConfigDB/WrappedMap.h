#ifndef DEF_WRAPPED_MAP
#define DEF_WRAPPED_MAP

#pragma warning (disable: 4786)
#include <map>
#include <string>

class NamedUnivType;

class WrappedMap
{
private:
	std::map<std::string, NamedUnivType *> m_map;
	std::map<std::string, NamedUnivType *>::iterator m_itr;
public:
	WrappedMap() {}
	void clear(){ m_map.clear(); }
	int size(){ return m_map.size(); }
	void erase(const char *p){ m_map.erase(p); }
	NamedUnivType *find(const char *p){
		std::map<std::string, NamedUnivType *>::iterator itr = m_map.find(p);
		return (itr != m_map.end()) ? itr->second : NULL;
	}
	const NamedUnivType *find(const char *p) const {
		std::map<std::string, NamedUnivType *>::const_iterator itr = m_map.find(p);
		return (itr != m_map.end()) ? itr->second : NULL;
	}
	bool erase(const NamedUnivType *p){
		for(m_itr = m_map.begin(); m_itr != m_map.end(); m_itr++){
			if(p == m_itr->second){
				m_map.erase(m_itr);
				return true;
			}
		}
		return false;
	}
	void add(const char *p, NamedUnivType *x){ m_map[p] = x; }
	NamedUnivType *getNext(bool begin = false){
		if(begin){
			m_itr = m_map.begin();
		}else{
			m_itr++;
		}
		return (m_itr != m_map.end()) ? m_itr->second : NULL;
	}

};

#endif
