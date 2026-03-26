# Terraform starter (AWS ECS/Fargate)

This starter provisions:
- ECS cluster
- API task definition + service
- Worker task definition + service
- CloudWatch log groups for both services

## Usage
```bash
cd infra/aws/terraform
cp terraform.tfvars.example terraform.tfvars
terraform init
terraform plan
terraform apply
```

## Notes
- This is intentionally minimal; integrate ALB, RDS, ElastiCache, and IAM task roles from your platform modules.
- Provide private subnet and security group IDs from your existing network stack.
