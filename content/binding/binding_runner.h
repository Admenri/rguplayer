#ifndef CONTENT_BINDING_BINDING_RUNNER_H_
#define CONTENT_BINDING_BINDING_RUNNER_H_

#include "base/worker/thread_worker.h"
#include "content/script/graphics.h"
#include "ui/widget/widget.h"

namespace content {

class BindingRunner final {
 public:
  struct BindingParams {
    std::string_view debug_output;

    base::WeakPtr<ui::Widget> window;
    base::Vec2i resolution;

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

  Graphics* GetScreen() { return graphics_screen_.get(); }

 private:
  void BindingMain(BindingParams initial_param);
  std::unique_ptr<base::ThreadWorker> binding_worker_;

  scoped_refptr<base::SequencedTaskRunner> binding_runner_;

  std::unique_ptr<Graphics> graphics_screen_;

  base::WeakPtrFactory<BindingRunner> weak_ptr_factory_{this};
};

}  // namespace content

#endif  // CONTENT_BINDING_BINDING_RUNNER_H_