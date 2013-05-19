function [M] = optical_flow(current, past)
    M = current - past;
