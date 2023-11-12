# Fragments
This repository showcases proof-of-concept implementations for image fragmentation using producer-consumer, observer, and timeout design patterns. Explore how images can be efficiently broken down into fragments, distributed among consumers, and later reconstructed using these patterns.

## Overview

The project explores the fragmentation and defragmentation of images using the producer-consumer pattern and observer pattern. It includes examples of how to use these patterns for image processing, demonstrating the flow of fragmented image data from a producer to a consumer, as well as implementing a timeout mechanism for handling delays.

## Table of Contents

- [Producer-Consumer](#producer-consumer)
- [Observer](#observer)
- [Timeout Handling](#timeout-handling)
- [Usage](#usage)

## Producer-Consumer

The producer-consumer pattern is implemented to fragment an image into smaller parts and distribute them among consumers for processing. The consumers then reconstruct the image from the received fragments.

## Observer

The observer pattern is used to decouple the image fragmenter (producer) from the image defragmenter (consumer). The defragmenter is notified whenever a new fragment arrives, and it reconstructs the image accordingly.

## Timeout Handling

To handle delays and potential packet loss, a timeout mechanism is introduced. The defragmenter waits for new fragments, and if a timeout occurs, it can request the retransmission of specific fragments.

## Usage

1. Clone the repository:

   ```bash
   git clone https://github.com/BonomoJoaoPaulo/Fragments.git
