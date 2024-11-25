//
// Created by 31435 on 2024/9/16.
//

#ifndef MYSQLMGR_H
#define MYSQLMGR_H

#include "../global.h"
#include "MysqlDao.h"
#include "../LogicSystem/Singleton.h"
#include "../Player/Player.h"

class MysqlMgr: public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    std::shared_ptr<Player> GetUser(int uid);
private:
    MysqlMgr(){};
    MysqlDao _dao;
};



#endif //MYSQLMGR_H
