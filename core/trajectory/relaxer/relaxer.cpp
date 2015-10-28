#include "relaxer.h"
#include "result_observer.h"

#include <boost/numeric/odeint.hpp>

relaxer::relaxer(network& net, const std::size_t max_time) :
  net_(net),
  net_dynamics_(net_),
  max_time_(max_time)
{
}

void relaxer::run()
{
  network_dynamics::state_type initial_state = net_.node_positions();
  boost::numeric::odeint::runge_kutta_fehlberg78<network_dynamics::state_type> rkf78;
  result_observer writer(std::cout);
  boost::numeric::odeint::integrate_n_steps(rkf78, net_dynamics_, initial_state, 0.0, 1.0, max_time_, writer);
}