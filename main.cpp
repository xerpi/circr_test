#include <iostream>
#include <circt/Dialect/HW/HWDialect.h>
#include <circt/Dialect/HW/HWInstanceGraph.h>
#include <circt/Dialect/HW/HWOps.h>
#include <mlir/IR/ImplicitLocOpBuilder.h>
#include <llvm/ADT/PostOrderIterator.h>

using namespace mlir;
using namespace circt;
using namespace hw;

#define EXPECT_EQ(a, b) \
  do { \
    if ((a) != (b)) \
      std::cerr << "Expect equal failed" << std::endl; \
  } while (0)

#define ASSERT_EQ(a, b) \
  assert((a) == (b))

int main()
{
  // Create a hw.module with no ports.
  MLIRContext context;
  context.loadDialect<HWDialect>();
  LocationAttr loc = UnknownLoc::get(&context);
  auto module = ModuleOp::create(loc);
  auto builder = ImplicitLocOpBuilder::atBlockEnd(loc, module.getBody());
  auto top = builder.create<HWModuleOp>(StringAttr::get(&context, "Top"),
                                        ArrayRef<PortInfo>{});

  builder.setInsertionPointToStart(top.getBodyBlock());
  auto wireTy = builder.getIntegerType(2);

  // Add two ports.
  SmallVector<std::pair<StringAttr, Value>> appendPorts;
  auto wireA = builder.create<ConstantOp>(wireTy, 0);
  appendPorts.emplace_back(builder.getStringAttr("a"), wireA);
  auto wireD = builder.create<ConstantOp>(wireTy, 1);
  appendPorts.emplace_back(builder.getStringAttr("d"), wireD);
  top.appendOutputs(appendPorts);

  SmallVector<std::pair<StringAttr, Value>> insertPorts;
  auto wireB = builder.create<ConstantOp>(wireTy, 2);
  insertPorts.emplace_back(builder.getStringAttr("b"), wireB);
  auto wireC = builder.create<ConstantOp>(wireTy, 3);
  insertPorts.emplace_back(builder.getStringAttr("c"), wireC);
  top.insertOutputs(1, insertPorts);

  auto ports = top.getAllPorts();
  ASSERT_EQ(ports.size(), 4u);

  EXPECT_EQ(ports[0].name, builder.getStringAttr("a"));
  EXPECT_EQ(ports[0].direction, PortDirection::OUTPUT);
  EXPECT_EQ(ports[0].type, wireTy);

  EXPECT_EQ(ports[1].name, builder.getStringAttr("b"));
  EXPECT_EQ(ports[1].direction, PortDirection::OUTPUT);
  EXPECT_EQ(ports[1].type, wireTy);

  EXPECT_EQ(ports[2].name, builder.getStringAttr("c"));
  EXPECT_EQ(ports[2].direction, PortDirection::OUTPUT);
  EXPECT_EQ(ports[2].type, wireTy);

  EXPECT_EQ(ports[3].name, builder.getStringAttr("d"));
  EXPECT_EQ(ports[3].direction, PortDirection::OUTPUT);
  EXPECT_EQ(ports[3].type, wireTy);

  auto output = cast<OutputOp>(top.getBodyBlock()->getTerminator());
  ASSERT_EQ(output->getNumOperands(), 4u);

  EXPECT_EQ(output->getOperand(0), wireA.getResult());
  EXPECT_EQ(output->getOperand(1), wireB.getResult());
  EXPECT_EQ(output->getOperand(2), wireC.getResult());
  EXPECT_EQ(output->getOperand(3), wireD.getResult());

  std::cout << "Done!" << std::endl;
}
