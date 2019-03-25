#include <Processors/IProcessor.h>

namespace DB
{

class IInflatingTransform : public IProcessor
{
protected:
    InputPort & input;
    OutputPort & output;

    Chunk current_chunk;
    bool has_input = false;
    bool generated = false;
    bool can_generate = false;

    virtual void consume(Chunk chunk) = 0;
    virtual bool canGenerate() = 0;
    virtual Chunk generate() = 0;

public:
    IInflatingTransform(Block input_header, Block output_header);

    Status prepare() override;
    void work() override;

    InputPort & getInputPort() { return input; }
    OutputPort & getOutputPort() { return output; }
};

}