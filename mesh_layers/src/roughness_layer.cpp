/*
 *  Copyright 2020, Sebastian Pütz
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  authors:
 *    Sebastian Pütz <spuetz@uni-osnabrueck.de>
 *
 */

#include "mesh_layers/roughness_layer.h"

#include <lvr2/algorithm/GeometryAlgorithms.hpp>
#include <lvr2/algorithm/NormalAlgorithms.hpp>
#include <pluginlib/class_list_macros.h>

PLUGINLIB_EXPORT_CLASS(mesh_layers::RoughnessLayer, mesh_map::AbstractLayer)

namespace mesh_layers {

bool RoughnessLayer::readLayer() {
  return true;
}

bool RoughnessLayer::writeLayer() {
    return true;
}

bool RoughnessLayer::computeLethals()
{
  ROS_INFO_STREAM("Compute lethals for \"" << layer_name << "\" (Roughness Layer) with threshold " << config.threshold );
  lethal_vertices.clear();
  for (auto vH : roughness) {
    if (roughness[vH] > config.threshold)
      lethal_vertices.insert(vH);
  }
  ROS_INFO_STREAM("Found " << lethal_vertices.size() << " lethal vertices.");
  return true;
}

float RoughnessLayer::threshold() { return config.threshold; }

bool RoughnessLayer::computeLayer() {
  ROS_INFO_STREAM("Computing roughness...");

  lvr2::DenseFaceMap<mesh_map::Normal> face_normals = map_ptr->faceNormals();

 /* face_normals = lvr2::calcFaceNormals(*mesh_ptr);
  ROS_INFO_STREAM("Computed " << face_normals.numValues()
                                << " face normals.");
*/
  lvr2::DenseVertexMap<mesh_map::Normal> vertex_normals = map_ptr->vertexNormals();
/*
  ROS_INFO_STREAM(
            "No vertex normals found in the given map file, computing them...");
  vertex_normals = lvr2::calcVertexNormals(*mesh_ptr, face_normals);
  */
  roughness = lvr2::calcVertexRoughness(*mesh_ptr, config.radius, vertex_normals);

  return computeLethals();
}

lvr2::VertexMap<float> &RoughnessLayer::costs() { return roughness; }

void RoughnessLayer::reconfigureCallback(mesh_layers::RoughnessLayerConfig &cfg, uint32_t level) {
  bool notify = false;

  ROS_INFO_STREAM("New roughness layer config through dynamic reconfigure.");
  if (first_config) {
    config = cfg;
    first_config = false;
    return;
  }

  if(config.threshold != cfg.threshold)
  {
    computeLethals();
    notify = true;
  }

  if(notify) notifyChange();

  config = cfg;
}

bool RoughnessLayer::initialize(const std::string &name) {
  first_config = true;
  reconfigure_server_ptr = boost::shared_ptr<
      dynamic_reconfigure::Server<mesh_layers::RoughnessLayerConfig>>(
      new dynamic_reconfigure::Server<mesh_layers::RoughnessLayerConfig>(
          private_nh));

  config_callback =
      boost::bind(&RoughnessLayer::reconfigureCallback, this, _1, _2);
  reconfigure_server_ptr->setCallback(config_callback);
  return true;
}

} /* namespace mesh_layers */
