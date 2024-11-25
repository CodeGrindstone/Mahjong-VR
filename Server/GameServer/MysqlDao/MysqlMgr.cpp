//
// Created by 31435 on 2024/9/16.
//

#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr() {}

std::shared_ptr<Player> MysqlMgr::GetUser(int uid) {
    return _dao.GetUser(uid);
}
