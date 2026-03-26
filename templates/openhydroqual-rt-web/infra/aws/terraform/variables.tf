variable "aws_region" {
  type        = string
  description = "AWS region for deployment"
  default     = "us-west-2"
}

variable "name_prefix" {
  type        = string
  description = "Resource name prefix"
  default     = "ohq-rt"
}

variable "vpc_id" {
  type        = string
  description = "Existing VPC ID"
}

variable "private_subnet_ids" {
  type        = list(string)
  description = "Private subnet IDs for ECS tasks"
}

variable "api_security_group_ids" {
  type        = list(string)
  description = "Security groups for API service"
}

variable "worker_security_group_ids" {
  type        = list(string)
  description = "Security groups for worker service"
}

variable "api_image" {
  type        = string
  description = "Container image for API service"
}

variable "worker_image" {
  type        = string
  description = "Container image for worker service"
}

variable "broker_url" {
  type        = string
  description = "Broker URL (ElastiCache or MQ endpoint)"
}

variable "result_backend" {
  type        = string
  description = "Result backend URL"
}

variable "ohquery_base_url" {
  type        = string
  description = "Internal OHQuery service endpoint"
}

variable "api_desired_count" {
  type        = number
  default     = 2
}

variable "worker_desired_count" {
  type        = number
  default     = 2
}
