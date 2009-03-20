/*
   Copyright 2008 - 2009 Sun Microsystems, Inc.

   The MySQL Connector/C++ is licensed under the terms of the GPL
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
 */

#include <cppconn/warning.h>

#include "resultset.h"
#include <sstream>
#include <stdlib.h>
#include <cppconn/resultset.h>
#include <cppconn/datatype.h>
#include <cppconn/connection.h>

namespace testsuite
{
namespace classes
{

void resultset::getInt()
{
  // Message for --verbose output
  logMsg("resultset::getInt - MySQL_ResultSet::getInt*");
  try
  {

    stmt.reset(con->createStatement());
    stmt->execute("DROP TABLE IF EXISTS test");
    stmt->execute("CREATE TABLE test(i integer, i_uns integer unsigned, b bigint, b_uns bigint unsigned)");

    int64_t r1_c1=L64(2147483646),
            r1_c2=L64(2147483650),
            r1_c3=L64(9223372036854775806),
            r2_c1=L64(2147483647),
            r2_c2=L64(2147483649),
            r2_c3=L64(9223372036854775807);

    uint64_t r1_c4=UL64(9223372036854775810),
            r2_c4=UL64(18446744073709551615);

    pstmt.reset(con->prepareStatement("INSERT INTO test(i, i_uns, b, b_uns) VALUES(?,?,?,?)"));

    ASSERT(pstmt.get() != NULL);
    pstmt->clearParameters();
    pstmt->setInt(1, static_cast<int> (r1_c1));
    pstmt->setInt64(2, r1_c2);
    pstmt->setInt64(3, r1_c3);
    pstmt->setUInt64(4, r1_c4);
    ASSERT_EQUALS(false, pstmt->execute());

    pstmt->clearParameters();
    pstmt->setInt(1, static_cast<int> (r2_c1));
    pstmt->setInt64(2, r2_c2);
    pstmt->setInt64(3, r2_c3);
    pstmt->setUInt64(4, r2_c4);
    ASSERT_EQUALS(false, pstmt->execute());

    pstmt.reset(con->prepareStatement("SELECT i, i_uns, b, b_uns FROM test"));
    ASSERT(pstmt.get() != NULL);
    ASSERT(pstmt->execute());

    res.reset(pstmt->getResultSet());
    checkResultSetScrolling(res);

    ASSERT(res->next());

    ASSERT_EQUALS((int64_t) res->getInt("i"), r1_c1);
    ASSERT_EQUALS((int64_t) res->getInt(1), r1_c1);

    ASSERT_EQUALS(res->getInt64("i_uns"), r1_c2);
    ASSERT_EQUALS(res->getInt64(2), r1_c2);

    ASSERT_EQUALS(res->getInt64("b"), r1_c3);
    ASSERT_EQUALS(res->getInt64(3), r1_c3);

    ASSERT_EQUALS(res->getUInt64("b_uns"), r1_c4);
    ASSERT_EQUALS(res->getUInt64(4), r1_c4);

    ASSERT(res->next());

    ASSERT_EQUALS((int64_t) res->getInt("i"), r2_c1);
    ASSERT_EQUALS((int64_t) res->getInt(1), r2_c1);

    ASSERT_EQUALS(res->getInt64("i_uns"), r2_c2);
    ASSERT_EQUALS(res->getInt64(2), r2_c2);

    ASSERT_EQUALS(res->getInt64("b"), r2_c3);
    ASSERT_EQUALS(res->getInt64(3), r2_c3);

    ASSERT_EQUALS(res->getUInt64("b_uns"), r2_c4);
    ASSERT_EQUALS(res->getUInt64(4), r2_c4);

    ASSERT_EQUALS(res->next(), false);

    res.reset(stmt->executeQuery("SELECT i, i_uns, b, b_uns FROM test"));
    checkResultSetScrolling(res);

    ASSERT(res->next());

    ASSERT_EQUALS((int64_t) res->getInt("i"), r1_c1);
    ASSERT_EQUALS((int64_t) res->getInt(1), r1_c1);

    ASSERT_EQUALS(res->getInt64("i_uns"), r1_c2);
    ASSERT_EQUALS(res->getInt64(2), r1_c2);

    ASSERT_EQUALS(res->getInt64("b"), r1_c3);
    ASSERT_EQUALS(res->getInt64(3), r1_c3);

    ASSERT_EQUALS(res->getUInt64("b_uns"), r1_c4);
    ASSERT_EQUALS(res->getUInt64(4), r1_c4);

    ASSERT(res->next());

    ASSERT_EQUALS((int64_t) res->getInt("i"), r2_c1);
    ASSERT_EQUALS((int64_t) res->getInt(1), r2_c1);

    ASSERT_EQUALS(res->getInt64("i_uns"), r2_c2);
    ASSERT_EQUALS(res->getInt64(2), r2_c2);

    ASSERT_EQUALS(res->getInt64("b"), r2_c3);
    ASSERT_EQUALS(res->getInt64(3), r2_c3);

    ASSERT_EQUALS(res->getUInt64("b_uns"), r2_c4);
    ASSERT_EQUALS(res->getUInt64(4), r2_c4);

    ASSERT_EQUALS(res->next(), false);

    stmt->execute("DROP TABLE IF EXISTS test");
  }
  catch (sql::SQLException &e)
  {
    logErr(e.what());
    logErr("SQLState: " + e.getSQLState());
    fail(e.what(), __FILE__, __LINE__);
  }
}

void resultset::getTypes()
{
  logMsg("resultset::getTypes - MySQL_ResultSet::get*");
  std::vector<columndefinition>::iterator it;
  std::stringstream msg;
  bool got_warning=false;
  ResultSet pres;
  std::string ps_value;
  std::string::size_type len_st;
  std::string::size_type len_ps;

  try
  {
    stmt.reset(con->createStatement());
    logMsg("... looping over all kinds of column types");
    for (it=columns.begin(); it != columns.end(); it++)
    {
      stmt->execute("DROP TABLE IF EXISTS test");
      msg.str("");
      msg << "CREATE TABLE test(dummy TIMESTAMP, id " << it->sqldef << ")";
      try
      {
        stmt->execute(msg.str());
        msg.str("");
        msg << "... testing " << it->sqldef << ", value = '" << it->value << "'";
        logMsg(msg.str());
      }
      catch (sql::SQLException &)
      {
        msg.str("");
        msg << "... skipping " << it->sqldef;
        logMsg(msg.str());
        continue;
      }

      msg.str("");
      msg << "INSERT INTO test(id) VALUES ('" << it->value << "')";
      stmt->execute(msg.str());

      res.reset(stmt->executeQuery("SELECT id, NULL FROM test"));
      checkResultSetScrolling(res);
      ASSERT(res->next());

      pstmt.reset(con->prepareStatement("SELECT id, NULL FROM test"));
      pstmt->clearParameters();
      pres.reset(pstmt->executeQuery());
      checkResultSetScrolling(pres);
      ASSERT(pres->next());


      if (it->check_as_string)
      {
        logMsg("... checking string value");
        if (it->as_string != res->getString("id"))
        {
          msg.str("");
          msg << "... expecting '" << it->as_string << "', got '" << res->getString("id") << "'";
          logMsg(msg.str());
          got_warning=true;
        }
      }
      ASSERT_EQUALS(res->getString("id"), res->getString(1));
      try
      {
        res->getString(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getString(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getString(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getString(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getDouble("id"), res->getDouble(1));
      try
      {
        res->getDouble(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getDouble(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getDouble(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getDouble(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getInt(1), res->getInt("id"));
      try
      {
        res->getInt(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getInt(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getInt(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getInt(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getUInt(1), res->getUInt("id"));
      try
      {
        res->getUInt(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getUInt(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getUInt(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getUInt(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getInt64("id"), res->getInt64(1));
      try
      {
        res->getInt64(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getInt64(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getInt64(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getInt64(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getUInt64("id"), res->getUInt64(1));
      try
      {
        res->getUInt64(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getUInt64(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getUInt64(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getUInt64(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();

      ASSERT_EQUALS(res->getBoolean("id"), res->getBoolean(1));
      try
      {
        res->getBoolean(0);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      try
      {
        res->getBoolean(3);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }

      res->beforeFirst();
      try
      {
        res->getBoolean(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->afterLast();
      try
      {
        res->getBoolean(1);
        FAIL("Invalid argument not detected");
      }
      catch (sql::InvalidArgumentException &)
      {
      }
      res->first();


      // Comparing prepared statement resultset and statement resultset
      if (it->check_as_string && (pres->getString("id") != res->getString("id")))
      {
        if (it->sqldef.find("ZEROFILL", 0) == std::string::npos)
        {
          ps_value=pres->getString("id");
          len_st=res->getString("id").length();
          len_ps=ps_value.length();
          if (len_ps > len_st)
          {
            // Something like 1.01000 vs. 1.01 ?
            std::string::size_type i;
            for (i=len_st; i < len_ps; i++)
            {
              if (ps_value.at(i) != '0')
                break;
            }
            if (i < (len_ps - 1))
            {
              got_warning=true;
              msg.str("");
              msg << "... \t\tWARNING - getString(), PS: '" << pres->getString("id") << "'";
              msg << ", Statement: '" << res->getString("id") << "'";
              logMsg(msg.str());
            }
          }
        }

      }
      // ASSERT_EQUALS(pres->getString("id"), res->getString("id"));

      if (!fuzzyEquals(pres->getDouble("id"), res->getDouble("id"), 0.001))
      {
        msg.str("");
        msg << "... \t\tWARNING - getDouble(), PS: '" << pres->getDouble("id") << "'";
        msg << ", Statement: '" << res->getDouble("id") << "'";
        msg << ", Difference: '" << (pres->getDouble("id") - res->getDouble("id")) << "'";
        logMsg(msg.str());
        got_warning=true;
      }
      //ASSERT_EQUALS(pres->getDouble("id"), res->getDouble("id"));

      if (pres->getInt("id") != res->getInt("id"))
      {
        msg.str("");
        msg << "... \t\tWARNING - getInt(), PS: '" << pres->getInt("id") << "'";
        msg << ", Statement: '" << res->getInt("id") << "'";
        logMsg(msg.str());
        got_warning=true;
      }
      // ASSERT_EQUALS(pres->getInt("id"), res->getInt("id"));

      if (pres->getUInt("id") != res->getUInt("id"))
      {
        msg.str("");
        msg << "... \t\tWARNING - getUInt(), PS: '" << pres->getUInt("id") << "'";
        msg << ", Statement: '" << res->getUInt("id") << "'";
        logMsg(msg.str());
        got_warning=true;
      }
      // ASSERT_EQUALS(pres->getUInt("id"), res->getUInt("id"));

      if (pres->getInt64("id") != res->getInt64("id"))
      {
        msg.str("");
        msg << "... \t\tWARNING - getInt64(), PS: '" << pres->getInt64("id") << "'";
        msg << ", Statement: '" << res->getInt64("id") << "'";
        logMsg(msg.str());
        got_warning=true;
      }
      // ASSERT_EQUALS(pres->getInt64("id"), res->getInt64("id"));

      if (pres->getUInt64("id") != res->getUInt64("id"))
      {
        msg.str("");
        msg << "... \t\tWARNING - getUInt64(), PS: '" << pres->getUInt64("id") << "'";
        msg << ", Statement: '" << res->getUInt64("id") << "'";
        logMsg(msg.str());
        got_warning=true;
      }
      // ASSERT_EQUALS(pres->getUInt64("id"), res->getUInt64("id"));

      ASSERT_EQUALS(pres->getBoolean("id"), res->getBoolean(1));

    }
    if (got_warning)
      FAIL("See warnings!");

    stmt->execute("DROP TABLE IF EXISTS test");
  }
  catch (sql::SQLException &e)
  {
    logErr(e.what());
    logErr("SQLState: " + e.getSQLState());
    fail(e.what(), __FILE__, __LINE__);
  }
}

void resultset::getTypesMinorIssues()
{
  logMsg("resultset::getTypesMinorIssues - MySQL_ResultSet::get*");
  std::vector<columndefinition>::iterator it;
  std::stringstream msg;  
  bool got_warning=false;
  bool got_minor_warning=false;
  ResultSet pres;
  std::string ps_value;
  std::string::size_type len_st;
  std::string::size_type len_ps;

  try
  {
    stmt.reset(con->createStatement());
    logMsg("... looping over all kinds of column types");
    for (it=columns.begin(); it != columns.end(); it++)
    {
      stmt->execute("DROP TABLE IF EXISTS test");
      msg.str("");
      msg << "CREATE TABLE test(dummy TIMESTAMP, id " << it->sqldef << ")";
      try
      {
        stmt->execute(msg.str());
        msg.str("");
        msg << "... testing " << it->sqldef << ", value = '" << it->value << "'";
        logMsg(msg.str());
      }
      catch (sql::SQLException &)
      {
        msg.str("");
        msg << "... skipping " << it->sqldef;
        logMsg(msg.str());
        continue;
      }

      msg.str("");
      msg << "INSERT INTO test(id) VALUES ('" << it->value << "')";
      stmt->execute(msg.str());

      res.reset(stmt->executeQuery("SELECT id, NULL FROM test"));
      checkResultSetScrolling(res);
      ASSERT(res->next());

      pstmt.reset(con->prepareStatement("SELECT id, NULL FROM test"));
      pstmt->clearParameters();
      pres.reset(pstmt->executeQuery());
      checkResultSetScrolling(pres);
      ASSERT(pres->next());

      if (it->check_as_string && (it->as_string != res->getString("id")))
      {
        msg.str("");
        msg << "... expecting '" << it->as_string << "', got '" << res->getString("id") << "'";
        logMsg(msg.str());
        got_warning=true;

      }

      // Comparing prepared statement resultset and statement resultset
      if (pres->getString("id") != res->getString("id"))
      {
        if (it->sqldef.find("ZEROFILL", 0) == std::string::npos)
        {
          bool is_minor=false;
          ps_value=pres->getString("id");
          len_st=res->getString("id").length();
          len_ps=ps_value.length();
          if (len_ps > len_st)
          {
            // Something like 1.01000 vs. 1.01 ?
            std::string::size_type i;
            for (i=len_st; i < len_ps; i++)
            {
              if (ps_value.at(i) != '0')
                break;
            }
            if (i < (len_ps - 1))
            {
              got_warning=true;
            }
            else
            {
              is_minor=true;
              got_minor_warning=true;
            }
          }
          if (!it->check_as_string)
          {
            is_minor=true;
            got_minor_warning=true;
          }
          else
          {
            got_warning=true;
          }
          msg.str("");
          if (is_minor)
          {
            msg << "... \t\tMINOR WARNING - getString(), PS: '" << pres->getString("id") << "'";
          }
          else
          {
            msg << "... \t\tWARNING - getString(), PS: '" << pres->getString("id") << "'";
          }
          msg << ", Statement: '" << res->getString("id") << "'";
          logMsg(msg.str());
        }

      }

    }
    if (got_warning)
      FAIL("See --verbose warnings!");

    if (got_minor_warning)
    {
      TODO("See MINOR WARNING when using --verbose");
      FAIL("TODO - see MINOR WARNING when using --verbose");
    }

    stmt->execute("DROP TABLE IF EXISTS test");
  }
  catch (sql::SQLException &e)
  {
    logErr(e.what());
    logErr("SQLState: " + e.getSQLState());
    fail(e.what(), __FILE__, __LINE__);
  }
}

void resultset::notImplemented()
{
  logMsg("resultset::notImplemented - MySQL_ResultSet::*");

  try
  {
    stmt.reset(con->createStatement());
    stmt->execute("DROP TABLE IF EXISTS test");
    stmt->execute("CREATE TABLE test(id INT)");
    ASSERT_EQUALS(1, stmt->executeUpdate("INSERT INTO test(id) VALUES (1)"));
    res.reset(stmt->executeQuery("SELECT id FROM test"));
    doNotImplemented();

    pstmt.reset(con->prepareStatement("SELECT id FROM test"));
    res.reset(pstmt->executeQuery());
    doNotImplemented();

    stmt->execute("DROP TABLE IF EXISTS test");
  }
  catch (sql::SQLException &e)
  {
    logErr(e.what());
    logErr("SQLState: " + e.getSQLState());
    fail(e.what(), __FILE__, __LINE__);
  }
}

void resultset::doNotImplemented()
{

  try
  {
    res->getWarnings();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->clearWarnings();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->insertRow();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->moveToCurrentRow();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->moveToInsertRow();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->refreshRow();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->rowDeleted();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->rowInserted();
  }
  catch (sql::MethodNotImplementedException)
  {
  }

  try
  {
    res->rowUpdated();
  }
  catch (sql::MethodNotImplementedException)
  {
  }
}

} /* namespace resultset */
} /* namespace testsuite */
