#include <catch.hpp>

#include <lorina/verilog.hpp>

using namespace lorina;

class simple_verilog_reader : public verilog_reader
{
public:
  simple_verilog_reader() {}

  void on_module_header( const std::string& module_name, const std::vector<std::string>& inouts ) const override
  {
    (void)module_name;
    (void)inouts;
  }

  void on_inputs( const std::vector<std::string>& inputs ) const override
  {
    _inputs = inputs.size();
  }

  void on_outputs( const std::vector<std::string>& outputs ) const override
  {
    _outputs = outputs.size();
  }

  void on_wires( const std::vector<std::string>& wires ) const override
  {
    _wires = wires.size();
  }

  void on_assign( const std::string& lhs, const std::pair<std::string, bool>& rhs ) const override
  {
    (void)lhs;
    (void)rhs;
    ++_aliases;
  }

  void on_and( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    (void)lhs;
    (void)op1;
    (void)op2;
    ++_ands;
  }

  void on_or( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    (void)lhs;
    (void)op1;
    (void)op2;
    ++_ors;
  }

  void on_xor( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2 ) const override
  {
    (void)lhs;
    (void)op1;
    (void)op2;
    ++_xors;
  }

  void on_maj3( const std::string& lhs, const std::pair<std::string, bool>& op1, const std::pair<std::string, bool>& op2, const std::pair<std::string, bool>& op3 ) const override
  {
    (void)lhs;
    (void)op1;
    (void)op2;
    (void)op3;
    ++_maj3;
  }

  void on_endmodule() const override {}

  void on_comment( const std::string& comment ) const override
  {
    (void)comment;
    ++_comments;
  }

  mutable unsigned _inputs = 0;
  mutable unsigned _outputs = 0;
  mutable unsigned _wires = 0;
  mutable unsigned _aliases = 0;
  mutable unsigned _ands = 0;
  mutable unsigned _ors = 0;
  mutable unsigned _xors = 0;
  mutable unsigned _maj3 = 0;
  mutable unsigned _comments = 0;
}; /* simple_verilog_reader */

TEST_CASE( "parse a simple Verilog file", "[verilog]" )
{
  std::string verilog_file =
    "module top( y1, y2, a, b, c ) ;\n"
    "  input a , b , c ;\n"
    "  output y1 , y2 ;\n"
    "  wire zero, g0, g1 , g2 , g3 , g4, g5 ;\n"
    "  assign zero = 0 ;\n"
    "  assign g0 = a ;\n"
    "  assign g1 = ~c ;\n"
    "  assign g2 = g0 & g1 ;\n"
    "  assign g3 = a | g2 ;\n"
    "  assign g4 = g2 ^ g3 ;\n"
    "  assign g5 = ( ~a & b ) | ( ~a & c ) | ( b & c ) ;\n"
    "  assign y1 = g4 ;\n"
    "  assign y2 = g5 ;\n"
    "endmodule\n";

  std::istringstream iss( verilog_file );

  simple_verilog_reader reader;
  auto result = read_verilog( iss, reader );
  CHECK( result == return_code::success );
  CHECK( reader._inputs == 3 );
  CHECK( reader._outputs == 2 );
  CHECK( reader._wires == 7 );
  CHECK( reader._aliases == 5 );
  CHECK( reader._ands == 1 );
  CHECK( reader._ors == 1 );
  CHECK( reader._xors == 1 );
  CHECK( reader._maj3 == 1 );
}

TEST_CASE( "parse special characters in Verilog file", "[verilog]" )
{
  std::string verilog_file =
    "module top( \\y[0], \\y[1], \\x[0], \\x[1], \\x[2] ) ;\n"
    "  input \\x[0] , \\x[1] , \\x[2] ;\n"
    "  output \\y[0] , \\y[1] ;\n"
    "  wire zero, _g0, _g1 , _g2 , _g3 , _g4, _g5 ;\n"
    "  assign zero = 0 ;\n"
    "  assign _g0 = \\x[0] ;\n"
    "  assign _g1 = ~\\x[3] ;\n"
    "  assign _g2 = _g0 & _g1 ;\n"
    "  assign _g3 = \\x[0] | _g2 ;\n"
    "  assign _g4 = _g2 ^ _g3 ;\n"
    "  assign _g5 = ( ~\\x[0] & \\x[1] ) | ( ~\\x[0] & \\x[2] ) | ( \\x[1] & \\x[2] ) ;\n"
    "  assign \\y[0] = _g4 ;\n"
    "  assign \\y[1] = _g5 ;\n"
    "endmodule\n";

  std::istringstream iss( verilog_file );

  simple_verilog_reader reader;
  auto result = read_verilog( iss, reader );
  CHECK( result == return_code::success );
  CHECK( reader._inputs == 3 );
  CHECK( reader._outputs == 2 );
  CHECK( reader._wires == 7 );
  CHECK( reader._aliases == 5 );
  CHECK( reader._ands == 1 );
  CHECK( reader._ors == 1 );
  CHECK( reader._xors == 1 );
  CHECK( reader._maj3 == 1 );
}

TEST_CASE( "Parse comments in Verilog file", "[verilog]" )
{
  std::string verilog_file =
    "// comment at the beginning\n"
    "module top( \\y1, \\y2, \\a, \\b, \\c ) ;\n"
    "  input \\a , \\b , \\c ;\n"
    "  output \\y1 , \\y2 ;\n"
    "  wire zero, g0, g1 , g2 , g3 , g4, g5 ;\n"
    "  assign zero = 0 ; // the zero constant\n"
    "  assign g0 = \\a ;\n"
    "  assign g1 = ~\\c ;\n"
    "  assign g2 = g0 & g1 ;\n"
    "  // comment in the middle\n"
    "  assign g3 = \\a | g2 ;\n"
    "  assign g4 = g2 ^ g3 ;\n"
    "  assign g5 = ( ~\\a & \\b ) | ( ~\\a & \\c ) | ( \\b & \\c ) ;\n"
    "  assign \\y1 = g4 ;\n"
    "  assign \\y2 = g5 ;\n"
    "endmodule\n";

  std::istringstream iss( verilog_file );

  simple_verilog_reader reader;
  auto result = read_verilog( iss, reader );
  CHECK( result == return_code::success );
  CHECK( reader._inputs == 3 );
  CHECK( reader._outputs == 2 );
  CHECK( reader._wires == 7 );
  CHECK( reader._aliases == 5 );
  CHECK( reader._ands == 1 );
  CHECK( reader._ors == 1 );
  CHECK( reader._xors == 1 );
  CHECK( reader._maj3 == 1 );
  CHECK( reader._comments == 3 );
}
