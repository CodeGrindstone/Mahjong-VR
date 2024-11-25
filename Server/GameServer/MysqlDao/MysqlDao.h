//
// Created by 31435 on 2024/9/16.
//

#ifndef MYSQLDAO_H
#define MYSQLDAO_H

#include "../global.h"
#include "../ConfigMgr/ConfigMgr.h"
#include "../Player/Player.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <iostream>


#include <mysql_driver.h>
#include <mysql_connection.h>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <iostream>

class MySqlPool {
public:
    // 构造函数：初始化连接池
    // 参数：
    //   url: 数据库连接的 URL
    //   user: 数据库用户名
    //   pass: 数据库密码
    //   schema: 要使用的数据库名称
    //   poolSize: 连接池的大小（即要创建的连接数）
    MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize)
        : url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize), b_stop_(false) {
        try {
            // 创建并初始化连接池
            for (int i = 0; i < poolSize_; ++i) {
                // 获取 MySQL 驱动实例
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

                // 创建数据库连接
                std::unique_ptr<sql::Connection> con(driver->connect(url_, user_, pass_));

                // 设置要使用的数据库
                con->setSchema(schema_);

                // 将连接添加到池中
                pool_.push(std::move(con));
            }
        } catch (sql::SQLException& e) {
            // 捕获并处理异常，例如连接失败等情况
            std::cerr << "MySQL pool initialization failed: " << e.what() << std::endl;
        }
    }

    // 获取一个数据库连接
    // 如果连接池为空，将会阻塞当前线程，直到有连接可用
    // 如果池已关闭，返回 nullptr
    std::unique_ptr<sql::Connection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);

        // 等待直到有连接可用或者池被关闭
        cond_.wait(lock, [this] {
            // 如果连接池被关闭，直接返回 true
            if (b_stop_) {
                return true;
            }
            // 如果连接池不为空，返回 true
            return !pool_.empty();
        });

        // 如果池已关闭，返回 nullptr
        if (b_stop_) {
            return nullptr;
        }

        // 从池中取出一个连接
        std::unique_ptr<sql::Connection> con(std::move(pool_.front()));
        pool_.pop(); // 从队列中移除该连接

        return con;
    }

    // 将连接归还到连接池中
    // 参数：
    //   con: 需要归还的连接
    void returnConnection(std::unique_ptr<sql::Connection> con) {
        std::unique_lock<std::mutex> lock(mutex_);

        // 如果池已关闭，直接返回
        if (b_stop_) {
            return;
        }

        // 将连接放回池中
        pool_.push(std::move(con));

        // 通知等待的线程有连接可用
        cond_.notify_one();
    }

    // 关闭连接池
    // 关闭后，所有等待获取连接的线程都会被唤醒，getConnection() 将返回 nullptr
    void Close() {
        b_stop_ = true;

        // 通知所有等待的线程连接池已关闭
        cond_.notify_all();
    }

    // 析构函数：清理连接池中的所有连接
    ~MySqlPool() {
        std::unique_lock<std::mutex> lock(mutex_);

        // 释放连接池中的所有连接
        while (!pool_.empty()) {
            pool_.pop();
        }
    }

private:
    std::string url_; // 数据库连接的 URL
    std::string user_; // 数据库用户名
    std::string pass_; // 数据库密码
    std::string schema_; // 要使用的数据库名称
    int poolSize_; // 连接池的大小
    std::queue<std::unique_ptr<sql::Connection>> pool_; // 存储连接的队列
    std::mutex mutex_; // 保护连接池的互斥锁
    std::condition_variable cond_; // 条件变量，用于线程同步
    std::atomic<bool> b_stop_; // 标记连接池是否已关闭
};


class MysqlDao
{
public:
    MysqlDao();
    ~MysqlDao();
    std::shared_ptr<Player> GetUser(int uid);
private:
    std::unique_ptr<MySqlPool> pool_;
};

#endif //MYSQLDAO_H
