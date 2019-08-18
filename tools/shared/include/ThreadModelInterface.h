//
// Created by yuan on 8/15/19.
//

#pragma once
#ifndef TOOLS_THREADMODELINTERFACE_H
#define TOOLS_THREADMODELINTERFACE_H

#include "ThreadJob.h"

/* provides a conceptual Input-only-based thread abstraction,
 * which the thread only takes an input and does not
 * share the output to other threads
 * example usage could be for:
 *      file writer thread, socket sender thread, etc.
 * the template field represents data that has
 * internal lock and is thread-safe */
template <typename ThreadSafeInput>
struct InputThreadInterface : virtual public ThreadJob<> {
protected:
    using InputType = ThreadSafeInput;

    ThreadSafeInput& inputData_;

    explicit InputThreadInterface(InputType& inputData) : inputData_(inputData) { }
};


/* provides a conceptual Output-only-based thread abstraction,
 * which the thread no input and produces output for other threads
 * example usage could be for:
 *      file reader thread, socket receiver thread, random num generator, etc.
 * the template field represents data that has
 * internal lock and is thread-safe */
template <typename ThreadSafeOutput>
struct OutputThreadInterface : virtual public ThreadJob<> {
protected:
    using OutputType = ThreadSafeOutput;

    ThreadSafeOutput& outputData_;

    /* optional batchSize to be used since writing
     * large number of single element into self-locking
     * outputData_ could slow down performance */
    const int batchSize_;

    explicit OutputThreadInterface(OutputType& outputData, const int batchSize) :
        outputData_(outputData), batchSize_(batchSize) { }
};


/* provides a conceptual Input-and-Output-based thread abstraction
 * where 2 template fields represent data that has
 * example usage:
 *      parser, data processing thread, etc.
 * internal lock and is thread-safe */
template <typename ThreadSafeInput, typename ThreadSafeOutput>
struct InputOutputThreadInterface :
        public InputThreadInterface<ThreadSafeInput>,
        public OutputThreadInterface<ThreadSafeOutput> {

    using ITI = InputThreadInterface<ThreadSafeInput>;
    using OTI = OutputThreadInterface<ThreadSafeOutput>;

protected:

    using InputType = typename ITI::InputType;
    using OutputType = typename OTI::OutputType;

    InputOutputThreadInterface(InputType& inputData, OutputType& outputData,
            const int batchSize = 128) :
            ITI(inputData), OTI(outputData, batchSize) { }
};

/* shorter name alias for InputOutputThreadInterface */
template <typename ThreadSafeInput, typename ThreadSafeOutput>
using IOThreadInterface = InputOutputThreadInterface<ThreadSafeInput, ThreadSafeOutput>;


#endif //TOOLS_THREADMODELINTERFACE_H
