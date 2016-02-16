#ifndef PIEKE_INCLUDE_CLASS_FACTORY_H_
#define PIEKE_INCLUDE_CLASS_FACTORY_H_

#include <string>
#include <map>

using std::string;
using std::map;
using std::make_pair;

/*
 * 使用这个typedef的前提是需要Object的派生类实现自己的无参构造函数
 * 如果没有无参构造函数的话当然是不能成功的
 * 这里返回一个空指针的原因是为了能将函数指针统一放入factoryMap_当中
 * 我在考虑使用boost::any来代替这里的void*,或者使用boost::function来代替这里的函数指针
 * Effective C++提出尽量少做转型操作，用static_cast这种新式转型来代替旧式转型
 */
typedef void* (*ObjectCreate_t)();

/*
 * 使用factory模式来模拟JAVA的反射机制
 */
class ClassFactory {
 public:
  //因为是singleton模式，所以这里不能够让用户随意创建一个ClassFacotory，所以构造函数应该放在protected当中去
  //只有static ClassFactory &Instance()能调用构造函数

  ClassFactory(const ClassFactory&) = delete;
  ClassFactory& operator=(const ClassFactory&) = delete;
  virtual ~ClassFactory();
  void Dump() const;

 public:

  static ClassFactory &Instance();

  /*
   * 关于为什么要把这里的返回值设置成为bool而不是void，原因是
   * 下面需要用到宏IMPL_CLASS_CREATE
   * 如果返回值是void的话，宏当中的AddObject就只剩这样的了：
   * ClassFactory::Instance().AddObject(#class_name, CreateClass##class_name)
   * 就没有前面的把返回值赋值给_bUnused的用法了
   * 倒不是说这个_Unused有多重要，看我给它起的名字是unused就知道它没有用了，是因为这个宏的使用一定是在函数的外部使用
   * 这样一来简单的调用AddObject就会有错：ClassFactory::Instance().AddObject(#class_name, CreateClass##class_name)
   * 因为C++不允许在全局的作用域当中调用函数。而写成这种形式：
   * static bool _##class_name##_Unused __attribute__((unused))= ClassFactory::Instance().AddObject(#class_name, CreateClass##class_name);
   * 这个__attribute__((unused))是编译器的内置宏，就是告诉编译器当这个函数没有被使用到的时候不要抛出警告，因为C++的规则就是定义没有调用的函数是有警告的
   * 所以上面那条语句其实等价于
   * static bool _Unused = ClassFactory::Instance().AddObject(#class_name, CreateClass##class_name);
   * 这就是初始化一个全局变量了，C++就判断合法
   * 所以下面的宏的用法包括这里的返回值为bool而不是void的用法就是这么个意思
   */
  bool AddObject(const string& className, ObjectCreate_t pCreate);
  bool AddObject(const char* className, ObjectCreate_t pCreate);
  void* GetObject(const string& className);
  void* GetObject(const char* className);

  map<string, ObjectCreate_t> GetMap() const;

 protected:
  ClassFactory();

 private:
  map<string, ObjectCreate_t> factoryMap_;
};

/*
 * 我犯了个错误，我最初将ClassFacotory的构造函数设为public，这就意味着用户能够自己构造一个ClassFactory，
 * 违反了singleton的原则,所以这里的实现不应该使用
 * inline ClassFactory& ClassFactoryInstance() {
 *   static ClassFactory sInstance;
 *   return sInstance;
 * }
 */

#define DECLARE_CLASS_CREATE(class_name)	    	\
	static void* CreateClass##class_name ();

#define IMPL_CLASS_CREATE(class_name)	            \
	static void* CreateClass##class_name (){	    \
		return (void*)(new class_name());			\
	};											    \
	static bool _##class_name##_Unused __attribute__((unused))= ClassFactory::Instance().AddObject(#class_name, CreateClass##class_name);

//#的作用是在class_name的左右两边都加上双引号，##的作用是连接两个字符串

#endif /* PIEKE_INCLUDE_CLASSS_FACTORY_H_ */
