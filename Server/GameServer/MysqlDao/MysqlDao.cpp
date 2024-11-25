//
// Created by 31435 on 2024/9/16.
//

#include "MysqlDao.h"

MysqlDao::MysqlDao()
{
    auto & cfg = ConfigMgr::getInstance();
    const auto& host = cfg["Mysql"]["Host"];
    const auto& port = cfg["Mysql"]["Port"];
    const auto& pwd = cfg["Mysql"]["Passwd"];
    const auto& schema = cfg["Mysql"]["Schema"];
    const auto& user = cfg["Mysql"]["User"];
    pool_.reset(new MySqlPool(host+":"+port, user, pwd,schema, 5));
}


MysqlDao::~MysqlDao(){
    pool_->Close();
}

std::shared_ptr<Player> MysqlDao::GetUser(int uid) 
{
    auto conn = pool_->getConnection();
    try{
        if(conn == nullptr)
            return nullptr;
        std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement("SELECT * FROM user WHERE uid = ?"));
        pstmt->setInt(1, uid);
        // 执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        // 检查是否有结果集
        if (!res || !res->next()) {
            // 如果没有结果，直接返回
            std::cout << "No results found." << std::endl;
            return nullptr; // 这里可以根据需要替换成合适的返回语句，例如 return nullptr; 或者其他
        }
        std::shared_ptr<Player> player_info = std::make_shared<Player>();
       // 如果有数据，继续处理
        do {
            std::cout << "name: " << res->getString("name") << std::endl;
            player_info->PlayerId = res->getInt("uid");
            player_info->PlayerName = res->getString("name");
        } while (res->next());
         
        return player_info; 
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        return nullptr;
    }
}