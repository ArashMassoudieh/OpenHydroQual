/*
 * Minimal ABI-v3 kernel used to verify parameter-liveness checks. The single
 * parameter intentionally changes only a modeled observation; state and mass
 * are constant. Rejecting it as inert is the regression this fixture catches.
 */
#define OHQ_KERNEL_NO_LOADER
#include "ohq_kernel.h"

struct ObservationOnlyState
{
    double parameter = 2.0;
    double time = 0.0;
    long steps = 0;
};

extern "C" {

int ohq_kernel_abi_version() { return OHQ_KERNEL_ABI_VERSION; }
const char* ohq_kernel_class_name() { return "ObservationOnlyKernel"; }

void* ohq_kernel_create() { return new ObservationOnlyState; }
void ohq_kernel_destroy(void* h) { delete static_cast<ObservationOnlyState*>(h); }
void ohq_kernel_initialize(void* h) { ohq_kernel_initialize_at(h, 0.0, 1.0); }
void ohq_kernel_initialize_at(void* h, double tstart, double)
{
    auto* s = static_cast<ObservationOnlyState*>(h);
    s->time = tstart;
    s->steps = 0;
}

int ohq_kernel_run_to(void* h, double t)
{
    auto* s = static_cast<ObservationOnlyState*>(h);
    s->time = t;
    ++s->steps;
    return 1;
}
int ohq_kernel_step_to(void* h, double t) { return ohq_kernel_run_to(h, t); }
int ohq_kernel_step(void* h)
{
    auto* s = static_cast<ObservationOnlyState*>(h);
    return ohq_kernel_run_to(h, s->time + 1.0);
}
double ohq_kernel_time(const void* h) { return static_cast<const ObservationOnlyState*>(h)->time; }
double ohq_kernel_simulation_start() { return 0.0; }
double ohq_kernel_simulation_end() { return 10.0; }

int ohq_kernel_n_parameters() { return 1; }
const char* ohq_kernel_parameter_name(int) { return "observation_scale"; }
double ohq_kernel_parameter(const void* h, int) { return static_cast<const ObservationOnlyState*>(h)->parameter; }
void ohq_kernel_set_parameter(void* h, int, double v) { static_cast<ObservationOnlyState*>(h)->parameter = v; }
void ohq_kernel_apply_parameters(void*) {}

int ohq_kernel_n_observations() { return 1; }
const char* ohq_kernel_observation_name(int) { return "scaled_observation"; }
int ohq_kernel_observation_count(const void* h, int)
{
    return static_cast<const ObservationOnlyState*>(h)->steps > 0 ? 2 : 0;
}
int ohq_kernel_observation_at(const void* h, int, int k, double* t, double* v)
{
    const auto* s = static_cast<const ObservationOnlyState*>(h);
    if (k < 0 || k >= ohq_kernel_observation_count(h, 0)) return 0;
    if (t) *t = k == 0 ? 0.0 : s->time;
    if (v) *v = s->parameter * (k + 1);
    return 1;
}
void ohq_kernel_clear_observations(void*) {}
void ohq_kernel_uniformize_observations(void*) {}

int ohq_kernel_n_objectives() { return 0; }
const char* ohq_kernel_objective_name(int) { return ""; }
int ohq_kernel_objective_count(const void*, int) { return 0; }
int ohq_kernel_objective_at(const void*, int, int, double*, double*) { return 0; }
void ohq_kernel_clear_objectives(void*) {}
void ohq_kernel_uniformize_objectives(void*) {}

int ohq_kernel_n_series() { return 0; }
const char* ohq_kernel_series_object(int) { return ""; }
const char* ohq_kernel_series_quantity(int) { return ""; }
int ohq_kernel_set_series(void*, const char*, const char*, const double*, const double*, int) { return 0; }
int ohq_kernel_set_precipitation(void*, const char*, const char*, const double*, const double*, const double*, int) { return 0; }

void ohq_kernel_export_state(const void*, double* storage, double*, int* limited, double* factor)
{
    if (storage) storage[0] = 7.0;
    if (limited) limited[0] = 0;
    if (factor) factor[0] = 1.0;
}
void ohq_kernel_import_state(void* h, double t, const double*, const double*, const int*, const double*)
{
    static_cast<ObservationOnlyState*>(h)->time = t;
}

int ohq_kernel_solution_failed(const void*) { return 0; }
double ohq_kernel_simulation_duration(const void*) { return 0.001; }
long ohq_kernel_step_count(const void* h) { return static_cast<const ObservationOnlyState*>(h)->steps; }
int ohq_kernel_last_iterations(const void*) { return 1; }
void ohq_kernel_reset_status(void* h) { static_cast<ObservationOnlyState*>(h)->steps = 0; }

int ohq_kernel_n_states() { return 1; }
double ohq_kernel_state(const void*, int) { return 7.0; }
const char* ohq_kernel_state_name(int) { return "constant_state"; }
int ohq_kernel_n_mass() { return 0; }
double ohq_kernel_mass(const void*, int) { return 0.0; }

} // extern "C"
