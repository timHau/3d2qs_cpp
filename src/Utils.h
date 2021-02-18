#ifndef INC_3D2QS_SUNC_CPP_UTILS_H
#define INC_3D2QS_SUNC_CPP_UTILS_H

namespace utils
{
	template<typename T>
	extern std::vector<std::pair<T, T>> cartesian_product(
			std::vector<T>& objs_a,
			std::vector<T>& objs_b
	)
	{
		std::vector<std::pair<T, T>> product;
		for (const T& i : objs_a)
		{
			for (const T& j : objs_b)
			{
				product.emplace_back(i, j);
			}
		}
		return product;
	}
}

#endif //INC_3D2QS_SUNC_CPP_UTILS_H
