#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#define N_FEATURES 2       
#define LEARNING_RATE 0.01  
#define MAX_EPOCHS 10000    
#define THRESHOLD 0.01      

// Training sample structure
typedef struct {
    double x[N_FEATURES];   
    double y;              
} Sample;

// Generate synthetic dataset on process 0
void generate_data(Sample *data, int n_samples) {
    srand(42); 
    for (int i = 0; i < n_samples; i++) {
        // Generate random features between -10 and 10
        data[i].x[0] = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
        data[i].x[1] = ((double)rand() / RAND_MAX) * 20.0 - 10.0;
        
        // True model: y = 3*x[0] - 5*x[1] + noise
        double noise = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        data[i].y = 3.0 * data[i].x[0] - 5.0 * data[i].x[1] + noise;
    }
}

// Compute prediction for one sample
double predict(double *weights, double *x) {
    double pred = 0.0;
    for (int i = 0; i < N_FEATURES; i++) {
        pred += weights[i] * x[i];
    }
    return pred;
}

// Compute local loss (MSE) and gradient
void compute_local_loss_and_gradient(Sample *local_data, int local_n, 
                                      double *weights, 
                                      double *local_loss, 
                                      double *local_gradient) {
    *local_loss = 0.0;
    for (int i = 0; i < N_FEATURES; i++) {
        local_gradient[i] = 0.0;
    }

    for (int i = 0; i < local_n; i++) {
        double prediction = predict(weights, local_data[i].x);
        
        double error = prediction - local_data[i].y;
        
        *local_loss += error * error;
        
        for (int j = 0; j < N_FEATURES; j++) {
            local_gradient[j] += 2.0 * error * local_data[i].x[j];
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Parse number of samples from command line
    int total_samples = 1000;
    if (argc > 1) {
        total_samples = atoi(argv[1]);
    }
    
    // Create MPI derived datatype for Sample structure
    MPI_Datatype sample_type;
    Sample dummy_sample;
    int blocklengths[2] = {N_FEATURES, 1};
    MPI_Datatype types[2] = {MPI_DOUBLE, MPI_DOUBLE};
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    
    MPI_Get_address(&dummy_sample, &base_address);
    
    MPI_Aint addr_x, addr_y;
    MPI_Get_address(&dummy_sample.x, &addr_x);
    MPI_Get_address(&dummy_sample.y, &addr_y);
    displacements[0] = addr_x - base_address;
    displacements[1] = addr_y - base_address;
    
    MPI_Type_create_struct(2, blocklengths, displacements, types, &sample_type);
    MPI_Type_commit(&sample_type);
    
    
    // Allocate dataset on process 0
    Sample *data = NULL;
    if (rank == 0) {
        data = (Sample *)malloc(total_samples * sizeof(Sample));
        generate_data(data, total_samples);
        printf("Dataset generated: %d samples, %d processes\n", total_samples, size);
    }
    
    // Determine how many samples each process gets
    int *sendcounts = NULL;
    int *displs = NULL;
    
    int base_count = total_samples / size;
    int remainder = total_samples % size;
    int local_n = base_count + (rank < remainder ? 1 : 0);
    
    if (rank == 0) {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        
        displs[0] = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = base_count + (i < remainder ? 1 : 0);
            if (i > 0) {
                displs[i] = displs[i-1] + sendcounts[i-1];
            }
        }
    }
    
    Sample *local_data = (Sample *)malloc(local_n * sizeof(Sample));
    
    // Scatter the dataset to all processes
    MPI_Scatterv(data, sendcounts, displs, sample_type,
                 local_data, local_n, sample_type,
                 0, MPI_COMM_WORLD);
    
    
    double weights[N_FEATURES];
    for (int i = 0; i < N_FEATURES; i++) {
        weights[i] = 0.0;  
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();
    
    // Gradient descent loop
    int epoch;
    double global_loss = 0.0;  
    
    for (epoch = 0; epoch < MAX_EPOCHS; epoch++) {
        // Compute local loss and gradient
        double local_loss;
        double local_gradient[N_FEATURES];
        
        compute_local_loss_and_gradient(local_data, local_n, weights, 
                                        &local_loss, local_gradient);
        
        // Aggregate loss across all processes 
        MPI_Allreduce(&local_loss, &global_loss, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        
        // Compute average loss (MSE)
        global_loss = global_loss / total_samples;
        
        // Aggregate gradients across all processes 
        double global_gradient[N_FEATURES];
        MPI_Allreduce(local_gradient, global_gradient, N_FEATURES, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        
        // Average the gradients and update weights
        for (int i = 0; i < N_FEATURES; i++) {
            global_gradient[i] = global_gradient[i] / total_samples;
            weights[i] = weights[i] - LEARNING_RATE * global_gradient[i];
        }
        
        if (rank == 0 && (epoch + 1) % 10 == 0) {
            printf("Epoch %4d | Loss (MSE): %f | w[0]: %.4f, w[1]: %.4f\n", 
                   epoch + 1, global_loss, weights[0], weights[1]);
        }
        
        // Early stopping: if global_loss < THRESHOLD, break
        if (global_loss < THRESHOLD) {
            break;
        }
    }
    
    // End timing
    end_time = MPI_Wtime();
    
    if (rank == 0) {
        if (epoch < MAX_EPOCHS) {
            printf("Early stopping at epoch %d - loss %f < %.1e\n", 
                   epoch, global_loss, THRESHOLD);
        }
        printf("Training time: %.3f seconds (MPI with %d processes)\n", 
               end_time - start_time, size);
        printf("Final weights: w[0] = %.4f, w[1] = %.4f\n", weights[0], weights[1]);
        printf("(Expected: w[0] = 3.0, w[1] = -5.0)\n");
    }
    
    // Cleanup
    free(local_data);
    if (rank == 0) {
        free(data);
        free(sendcounts);
        free(displs);
    }
    
    MPI_Type_free(&sample_type);
    
    MPI_Finalize();
    return 0;
}
