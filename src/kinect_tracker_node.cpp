#include <ros/ros.h>
#include <vector>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_ros/point_cloud.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>
#include <std_msgs/String.h>


class KinectPositionTracker
{
public:
	ros::NodeHandle nh;
	//publisher for all the postion information
	ros::Publisher position_pub;
	ros::Subscriber reset;
        tf::TransformListener listener;
        //sensor_msgs::PointCloud2 pos_info;
       double init_x,init_y,init_z;	
       
       bool first_msg_received;
	KinectPositionTracker(){
		//position_pub = nh.advertise<sensor_msgs::PointCloud2>("/position_inf",1);
		position_pub = nh.advertise<geometry_msgs::Twist>("/position_inf",1);
		reset = nh.subscribe("/reset",1,&KinectPositionTracker::resetCallback,this);
		first_msg_received = false;
		
	}

	void resetCallback(const std_msgs::String::ConstPtr& reset_msg){
		if(reset_msg != NULL){
			first_msg_received = false; 
		}
	}

};


int main(int agrc, char** argv){
	ros::init(agrc, argv, "kinect_tracker_node");
	KinectPositionTracker tracker;
	ros::Rate rate(10.0);
	geometry_msgs::Twist msg;
	while(ros::ok()){
		//receive the reset message
		ros::spinOnce();
		tf::StampedTransform transform_hand;
		tf::StampedTransform transform_elbow;
		tf::StampedTransform transform_shoulder;
		bool all_right;
		try{
		        all_right = true;
			tracker.listener.lookupTransform("/openni_depth_frame", "/left_hand_1",ros::Time(0),transform_hand);
			tracker.listener.lookupTransform("/openni_depth_frame", "/left_elbow_1",ros::Time(0),transform_elbow);
			tracker.listener.lookupTransform("/openni_depth_frame", "/left_shoulder_1",ros::Time(0),transform_shoulder);
		}
		catch(tf::TransformException ex){
			//ROS_ERROR("%s", ex.what());
			ros::Duration(1.0).sleep();
			all_right = false;
	        }
		if(!tracker.first_msg_received and all_right){
			tracker.first_msg_received = true;
			ROS_INFO("First position message received\n");
			tracker.init_x = transform_hand.getOrigin().z();
		        tracker.init_y = transform_hand.getOrigin().x();
		        tracker.init_z = transform_hand.getOrigin().y();
		}
		if(tracker.first_msg_received){
		//hand information point
		double hand_x, hand_y, hand_z;
                double theta_x, theta_y, theta_z;
	        hand_x = transform_hand.getOrigin().z() - tracker.init_x;
		hand_y = transform_hand.getOrigin().x() - tracker.init_y;
		hand_z = transform_hand.getOrigin().y() - tracker.init_z;
                //points.push_back(pcl::PointXYZ(hand_x,hand_y,hand_z));
                /*theta_x = transform_hand.getRotation().z();
                theta_y = transform_hand.getRotation().x();
                theta_z = transform_hand.getRotation().y(); */ 
                msg.linear.x = hand_x;
		msg.linear.y = hand_y;
		msg.linear.z = hand_z;
                transform_hand.getBasis().getRPY(theta_x,theta_y,theta_z);
            	msg.angular.x = theta_x;
            	msg.angular.y = theta_y;
           	msg.angular.z = theta_z;
		tracker.position_pub.publish(msg);
		ROS_INFO("hand position: %f, %f, %f \n",hand_x, hand_y, hand_z);
                //ROS_INFO("hand orientation: %f, %f, %f \n",theta_x, theta_y, theta_z);
		}
		//shoulder information point
		/*geometry_msgs::Point point_shoulder;
		double shoulder_x, shoulder_y, shoulder_z;
		shoulder_x = transform_shoulder.getOrigin().z();
		shoulder_y = transform_shoulder.getOrigin().x();
		shoulder_z = transform_shoulder.getOrigin().y();
		points.push_back(pcl::PointXYZ(shoulder_x,shoulder_y,shoulder_z));

		pcl::toROSMsg(points,tracker.pos_info);
		tracker.position_pub.publish(tracker.pos_info);
                points.points.clear(); */
	}
        return 0;


}





