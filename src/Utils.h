#ifndef INC_3D2QS_SUNC_CPP_UTILS_H
#define INC_3D2QS_SUNC_CPP_UTILS_H

#include "Object.h"

namespace utils {
    std::vector<std::pair<Object, Object>>
    cartesian_product(const std::vector<Object>& objs_a, const std::vector<Object>& objs_b) {
        std::vector<std::pair<Object, Object>> product;
        for (const Object& i : objs_a) {
            for (const Object& j : objs_b) {
                product.emplace_back(i, j);
            }
        }
        return product;
    }

}

#endif //INC_3D2QS_SUNC_CPP_UTILS_H
