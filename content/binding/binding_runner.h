#ifndef CONTENT_BINDING_BINDING_RUNNER_H_
#define CONTENT_BINDING_BINDING_RUNNER_H_

#include "base/worker/thread_worker.h"

namespace content {

class BindingRunner final {
 public:
  struct BindingParams {
    std::string_view debug_output;

    BindingParams() = default;
    BindingParams(const BindingParams&) = delete;
    BindingParams(BindingParams&&) = default;
  };

  BindingRunner();
  ~BindingRunner();

  BindingRunner(const BindingRunner&) = delete;
  BindingRunner& operator=(const BindingRunner&) = delete;

  scoped_refptr<base::SequencedTaskRunner> GetBindingRunnerTask();

  void InitializeBindingInterpreter();
  void PostBindingBoot(BindingParams initial_param);

 private:
  void BindingMain(BindingParams initial_param);
  std::unique_ptr<base::ThreadWorker> binding_worker_;

  scoped_refptr<base::SequencedTaskRunner> binding_runner_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_BINDING_BINDING_RUNNER_H_