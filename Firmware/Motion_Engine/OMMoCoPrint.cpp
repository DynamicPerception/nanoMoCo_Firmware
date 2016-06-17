// 
// 
// 

#include "OMMoCoPrint.h"

OMMoCoPrintClass::OMMoCoPrintClass(OMMoCoNode *node){
	m_node = node;
}

void OMMoCoPrintClass::init()
{


}

size_t OMMoCoPrintClass::write(uint8_t data){
	m_node->write(data);
}

