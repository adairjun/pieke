#include "pieke/class_factory.h"

ClassFactory::ClassFactory() {
}

ClassFactory::~ClassFactory() {
}

void ClassFactory::Dump() const {
  printf("\n=====ClassFactory Dump START ========== \n");
  int count = 0;
  for (auto it = factoryMap_.begin(); it!=factoryMap_.end(); ++it) {
    printf("count==%d ", count);
    printf("className=%s ", it->first.c_str());
    printf("CreateObject_t=%p ", it->second);
    printf("\n");
    ++count;
  }
  printf("\n===ClassFactory DUMP END ============\n");
}

ClassFactory& ClassFactory::Instance() {
  static ClassFactory sInstance;
  return sInstance;
}

bool ClassFactory::AddObject(const string& className, ObjectCreate_t pCreate) {
  factoryMap_.insert(make_pair(className, pCreate));
  return true;
}

bool ClassFactory::AddObject(const char* className, ObjectCreate_t pCreate) {
  string class_name(className);
  factoryMap_.insert(make_pair(class_name, pCreate));
  return true;
}

void* ClassFactory::GetObject(const string& className) {
  if (factoryMap_.count(className) == 1) {
    //这里就调用CreateObject_t()
    return factoryMap_[className]();
  } else {
	return NULL;
  }
}

void* ClassFactory::GetObject(const char* className) {
  string class_name(className);
  return GetObject(class_name);
}

/*
 * 我在考虑这里要不要使用std::move(factoryMap_)
 */
map<string, ObjectCreate_t> ClassFactory::GetMap() const {
  return factoryMap_;
}
