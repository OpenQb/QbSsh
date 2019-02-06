#ifndef QBOOGMAP_P_H
#define QBOOGMAP_P_H

#include <QList>

namespace QbCore {

template <class TKey,class TValue>
class QbOOGMap
{
private:
    QList<TKey> m_colOne;
    QList<TValue> m_colTwo;

public:
    QbOOGMap(){

    }

    void prepend(const TKey &key, const TValue &value){
        this->m_colOne.prepend(key);
        this->m_colTwo.prepend(value);
    }

    void append(const TKey &key, const TValue &value){
        this->m_colOne.append(key);
        this->m_colTwo.append(value);
    }

    void remove(const qint32 &index)
    {
        if(index<0 || index>=this->m_colOne.size()) return;
        this->m_colOne.removeAt(index);
        this->m_colTwo.removeAt(index);
    }

    TValue get(const qint32 &index){
        return this->m_colTwo.at(index);
    }

    bool isKeyExists(const TKey &key){
        return this->m_colOne.indexOf(key) != -1;
    }

    bool isValueExists(const TValue &value){
        return this->m_colTwo.indexOf(value) != -1;
    }

    int indexOf(const TKey &key){
        return this->m_colOne.indexOf(key);
    }

    int indexOfValue(const TValue &value){
        return this->m_colTwo.indexOf(value);
    }

    void removeByKey(const TKey &key){
        qint32 index = this->m_colOne.indexOf(key);
        if(index!=-1){
            this->m_colOne.removeAt(index);
            this->m_colTwo.removeAt(index);
        }
    }

    void removeByValue(const TValue &value){
        qint32 index = this->m_colTwo.indexOf(value);
        if(index!=-1){
            this->m_colOne.removeAt(index);
            this->m_colTwo.removeAt(index);
        }
    }

    TValue getByKey(const TKey &key){
        qint32 index = this->m_colOne.indexOf(key);
        return this->m_colTwo.at(index);
    }

    TKey getByValue(const TValue &value){
        qint32 index = this->m_colTwo.indexOf(value);
        return this->m_colOne.at(index);
    }

    qint32 length(){
        return this->m_colOne.length();
    }

    qint32 size(){
        return this->m_colTwo.length();
    }

    bool isConsistent(){
        return this->m_colOne.size() == this->m_colTwo.size();
    }

    bool isValidIndex(const qint32 &index){
        return index>=0 || index<this->m_colOne.size();
    }

    void clear(){
        this->m_colOne.clear();
        this->m_colTwo.clear();
    }

};

}
#endif // QBOOGMAP_P_H
