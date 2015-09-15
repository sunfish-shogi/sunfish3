/* CodeGenerator.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_CODEGENERATOR__
#define SUNFISH_CODEGENERATOR__

#if !defined(NDEBUG)

namespace sunfish {

class CodeGenerator {
private:

public:

  void generateZobrist();
  void generateMoveTable();

};

} // namespace sunfish

#endif // !defined(NDEBUG)

#endif // SUNFISH_CODEGENERATOR__
