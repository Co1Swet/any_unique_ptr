#include"any_unique_ptr/any_unique_ptr.hpp"
#include<iostream>

class CouldPrint {
	
public:
	int a = 0;

	CouldPrint() = default;
	CouldPrint(const CouldPrint&) = default;
	CouldPrint& operator=(const CouldPrint&) = delete;
	CouldPrint(CouldPrint&&) noexcept = default;
	CouldPrint& operator=(CouldPrint&&) noexcept = default;
	void print() const {
		std::cout << "CouldPrint instance at " << this << std::endl;
	}
	//自动析构测试
	~CouldPrint() {
		std::cout << "CouldPrint instance at " << this << " is being destroyed." << std::endl;
	}
};

int main() {
	MulNXB::any_unique_ptr ptr1 = MulNXB::make_any_unique<CouldPrint>();
	//假设中间进行了一系列传递，而ptr的类型信息已经在中间被模糊
	//但是，即使中间件不知道类型信息，两端确实应当知道类型信息的
	//我们对此进行模仿
	MulNXB::any_unique_ptr ptrMid = std::move(ptr1);
	MulNXB::any_unique_ptr ptrMid2 = std::move(ptrMid);

	//好了，现在已经重重转移了，但我们可以使用它

	//我们甚至可以克隆它（要求原类有拷贝构造函数）
	{
		MulNXB::any_unique_ptr ptrcopy = ptrMid2.clone();
		CouldPrint* cp = ptrcopy.get<CouldPrint>();
		if (cp != nullptr) {//类型安全获取
			cp->print();
		}
		
	}
	//我们也可以移动它
	{
		MulNXB::any_unique_ptr ptr2 = std::move(ptrMid2);
		CouldPrint* cp = ptr2.get<CouldPrint>();
		if (cp != nullptr) {//类型安全获取
			cp->print();
		}
	}

	system("pause");

	return 0;
}