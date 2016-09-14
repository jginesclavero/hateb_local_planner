#ifndef EDGE_HUMAN_ROBOT_H_
#define EDGE_HUMAN_ROBOT_H_

#include <teb_local_planner/obstacles.h>
#include <teb_local_planner/robot_footprint_model.h>
#include <teb_local_planner/g2o_types/vertex_pose.h>
#include <teb_local_planner/g2o_types/penalties.h>
#include <teb_local_planner/teb_config.h>

#include "g2o/core/base_unary_edge.h"

namespace teb_local_planner
{

class EdgeHumanRobot : public g2o::BaseBinaryEdge<1, double, VertexPose, VertexPose> {
public:
    EdgeHumanRobot() {
        this->setMeasurement(0.);
        _vertices[0] = _vertices[1] = NULL;
    }

    virtual ~EdgeHumanRobot() {
        for (unsigned int i=0; i<2; i++) {
            if(_vertices[i])
                _vertices[i]->edges().erase(this);
        }
    }

    void computeError() {
        ROS_ASSERT_MSG(cfg_ && robot_model_ && human_model_, "You must call "
                       "setTebConfig(), setRobotModel() and setHumanModel() "
                       "on EdgeHumanRobot()");
        const VertexPose* robot_bandpt = static_cast<const VertexPose*>(_vertices[0]);
        const VertexPose* human_bandpt = static_cast<const VertexPose*>(_vertices[1]);

        static_cast<PointObstacle*>(obs_)->setCentroid(human_bandpt->x(), human_bandpt->y());

       double dist = robot_model_->calculateDistance(robot_bandpt->pose(), obs_) - radius_;
        _error[0] = penaltyBoundFromBelow(dist, cfg_->human.min_human_dist,
                                          cfg_->optim.penalty_epsilon);

        ROS_ASSERT_MSG(std::isfinite(_error[0]),
                                     "EdgeHumanRobot::computeError() _error[0]=%f\n",
                                     _error[0]);
    }

    ErrorVector& getError() {
        computeError();
        return _error;
    }

    virtual bool read(std::istream& is) {
        // is >> _measurement[0];
        return true;
    }

    virtual bool write(std::ostream& os) const {
        // os << information()(0,0) << " Error: " << _error[0] << ", Measurement:"
        //    << _measurement[0];
        return os.good();
    }

    void setRobotModel(const BaseRobotFootprintModel* robot_model) {
        robot_model_ = robot_model;
    }

    void setHumanModel(const CircularRobotFootprint* human_model) {
        human_model->getRadius(radius_);
        // human_model_ = human_model;
    }

    void setTebConfig(const TebConfig& cfg) {
        cfg_ = &cfg;
    }

    void setParameters(const TebConfig& cfg,
                       const BaseRobotFootprintModel* robot_model,
                       const CircularRobotFootprint* human_model) {
        cfg_ = &cfg;
        robot_model_ = robot_model;
        human_model->getRadius(radius_);
        // human_model_ = human_model;
    }

protected:
    const TebConfig* cfg_;
    const BaseRobotFootprintModel* robot_model_;
    const CircularRobotFootprint* human_model_;
    Obstacle* obs_ = new PointObstacle();
    double radius_;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

} // end namespace

#endif