#include <gtest/gtest.h>

#include <sys/math/models.hpp>
#include <sys/math/filtering.hpp>
#include <sys/math/states.hpp>
#include <Eigen/Core>


using namespace Eigen;
using namespace sys;

typedef math::models::SCart3DQuat<> StateDescription;
typedef StateDescription states;
typedef math::GaussianFilter<StateDescription> Filter;
typedef math::models::ConstantVelocities6D<StateDescription> MotionModel;
typedef math::EKF Algorithm;


class EKFTests : public ::testing::Test {
    public:
        Filter filter;
        EKFTests()
        {
            states::initialize(filter);
        }
};

TEST_F(EKFTests, TimeUpdateStandStill) {
    auto reference = filter.state;
    Algorithm::timeUpdate<MotionModel>(filter, 1.0);
    EXPECT_TRUE((reference-filter.state).norm() < 1e-3) << reference.transpose() << " = " << filter.state.transpose() << "  [ " << (reference-filter.state).transpose() << " ]";
}

TEST_F(EKFTests, TimeUpdateMovingPositiveX) {
    filter.state[states::vx] = 1.0;
    auto reference = filter.state;
    reference[states::x] = 1.0;

    Algorithm::timeUpdate<MotionModel>(filter, 1.0);
    EXPECT_TRUE((reference-filter.state).norm() < 1e-3) << reference.transpose() << " = " << filter.state.transpose() << "  [ " << (reference-filter.state).transpose() << " ]";
}

TEST_F(EKFTests, TimeUpdateMovingNegativeX) {
    filter.state[states::vx] = -1.0;
    auto reference = filter.state;
    reference[states::x] = -1.0;

    Algorithm::timeUpdate<MotionModel>(filter, 1.0);
    EXPECT_TRUE((reference-filter.state).norm() < 1e-3) << reference.transpose() << " = " << filter.state.transpose() << "  [ " << (reference-filter.state).transpose() << " ]";
}

TEST_F(EKFTests, TimeUpdateRotationPositiveX) {
    filter.state[states::wx] = 1.0;
    auto reference = filter.state;
    reference[states::qw] = 0;
    reference[states::qx] = -1.0;

    Algorithm::timeUpdate<MotionModel>(filter, M_PI);
    EXPECT_TRUE((reference-filter.state).norm() < 1e-7) << reference.transpose() << " = " << filter.state.transpose();
}

TEST_F(EKFTests, TimeUpdateRepeatedRotationPositiveX) {
    filter.state[states::wx] = 1.0;
    auto reference = filter.state;
    reference[states::qw] = 0;
    reference[states::qx] = -1.0;

    for(int i = 0; i < 100; ++i) {
        Algorithm::timeUpdate<MotionModel>(filter, M_PI/100);
    }
    EXPECT_TRUE((reference-filter.state).norm() < 1e-5) << reference.transpose() << " = " << filter.state.transpose();
}


TEST_F(EKFTests, MeasurementUpdateGPS) {
    auto reference = filter.state;
    reference[states::x] = reference[states::y] = reference[states::z] = 0.5;

    math::GaussianMeasurement<sys::math::models::Gps<StateDescription>> m;
    m.z << 1, 1, 1;
    m.R.setIdentity();

    Algorithm::measurementUpdate<>(filter, m);
    EXPECT_TRUE((reference-filter.state).norm() < 1e-5) << reference.transpose() << " = " << filter.state.transpose();
}