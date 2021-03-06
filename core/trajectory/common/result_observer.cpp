#include "result_observer.h"

#include <iomanip>

stream_dumper::stream_dumper(format_type format, std::ostream& out) :
  format_(format),
  out_(out)
{
}

void stream_dumper::process(const state_type& r, const double t)
{
  switch(format_)
  {
    case format_type::gnuplot:
      format_for_gnuplot(r, t);
      break;
    case format_type::raw:
      format_for_raw(r, t);
      break;
    default:
      break;
  }
}

void stream_dumper::format_for_raw(const state_type& r, const double t)
{
  for(std::size_t i = 0; i < r.size(); ++i)
  {
    out_ << r[i] << std::endl;
  }
  out_ << std::endl;
}

void stream_dumper::format_for_gnuplot(const state_type& r, const double t)
{
  for(std::size_t i = 0; i < r.size(); ++i)
  {
    out_ << std::setw(15) << std::left << r[i][0] << std::setw(15) << std::left << r[i][1] << std::setw(15) << r[i][2];
    out_ << std::endl;
  }
  out_ << '\n' << std::endl;
}

trajectory_dumper::trajectory_dumper(std::ostream& out, const network::node_positions_type& initial_positions, const node_chooser::node_numbers_type& nodes, const std::size_t step) :
  out_(out),
  initial_positions_(initial_positions),
  nodes_(nodes),
  step_(step)
{
}

void trajectory_dumper::process(const state_type& r, const double t)
{
  static std::size_t counter = 0;
  if(0 == counter++ % step_)
  {
    std::size_t i1 = nodes_[0];
    std::size_t i2 = nodes_[1];
    std::size_t i3 = nodes_[2];

    double current_dist = utils::distance(r[i1], r[i2]);
    double initial_dist = utils::distance(initial_positions_[i1], initial_positions_[i2]);
    double val1 = (current_dist-initial_dist)/initial_dist;

    current_dist = utils::distance(r[i1], r[i3]);
    initial_dist = utils::distance(initial_positions_[i1], initial_positions_[i3]);
    double val2 = (current_dist-initial_dist)/initial_dist;

    current_dist = utils::distance(r[i2], r[i3]);
    initial_dist = utils::distance(initial_positions_[i2], initial_positions_[i3]);
    double val3 = (current_dist-initial_dist)/initial_dist;

    out_ << val1 << " " << val2 << " " << val3 << std::endl;
  }
}

stability_checker::stability_checker(const network::node_positions_type& initial_positions, const node_chooser::node_numbers_type& nodes) :
  initial_positions_(initial_positions),
  nodes_(nodes)
{
}

void stability_checker::process(const state_type& r, const double t)
{
  std::size_t i1 = nodes_[0];
  std::size_t i2 = nodes_[1];
  std::size_t i3 = nodes_[2];
  if(utils::distance(initial_positions_[i1], r[i1]) < 0.00001 &&
    utils::distance(initial_positions_[i2], r[i2]) < 0.00001 &&
    utils::distance(initial_positions_[i3], r[i3]) < 0.00001)
  {
    throw std::exception();
  }
}

composite_result_observer::composite_result_observer()
{
}

void composite_result_observer::process(const state_type& r, const double t)
{
  for(auto it = observers_.begin(); it != observers_.end(); ++it)
  {
    (*it)->process(r, t);
  }
}

void composite_result_observer::add_result_observer(const std::shared_ptr<result_observer>& observer)
{
  observers_.push_back(observer);
}

bg_thread_handler::bg_thread_handler() :
  pool_(10)
{
}

bg_thread_handler::~bg_thread_handler()
{
  for(auto&& res : futures_)
  {
    res.get();
  }
}

void bg_thread_handler::add_future(std::future<void>&& f)
{
  futures_.emplace_back(std::move(f));
}

result_observer_wrapper::result_observer_wrapper(const network_dynamics_wrapper& dynamics) :
  dynamics_(dynamics),
  bg_handler_(new bg_thread_handler())
{
}

void result_observer_wrapper::operator()(const state_type& r, const double t)
{
  //bg_handler_->add_future(std::async(std::launch::async,
  //  [&r, t, this]
  //  {
      dynamics_.prepare_for_step();
      if(observer_)
      {
        observer_->process(r, t);
      }
  //  }));
}

void result_observer_wrapper::set_result_observer(const std::shared_ptr<result_observer>& observer)
{
  observer_ = observer;
}
