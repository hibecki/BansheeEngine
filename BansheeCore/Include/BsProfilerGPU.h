#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	/**
	* @brief	Contains various profiler statistics about a single GPU profiling sample
	*/
	struct GPUProfileSample
	{
		String name; /**< Name of the sample for easier identification. */
		float timeMs; /**< Time in milliseconds it took to execute the sampled block. */

		UINT32 numDrawCalls; /**< Number of draw calls that happened. */
		UINT32 numRenderTargetChanges; /**< How many times was render target changed. */
		UINT32 numPresents; /**< How many times did a buffer swap happen on a double buffered render target. */
		UINT32 numClears; /**< How many times was render target cleared. */

		UINT32 numVertices; /**< Total number of vertices sent to the GPU. */
		UINT32 numPrimitives; /**< Total number of primitives sent to the GPU. */
		UINT32 numDrawnSamples; /**< Number of samples drawn by the GPU. */

		UINT32 numBlendStateChanges; /**< How many times did the blend state change. */
		UINT32 numRasterizerStateChanges; /**< How many times did the rasterizer state change. */
		UINT32 numDepthStencilStateChanges; /**< How many times did the depth stencil state change. */

		UINT32 numTextureBinds; /**< How many times was a texture bound. */
		UINT32 numSamplerBinds; /**< How many times was a sampler bound. */
		UINT32 numVertexBufferBinds; /**< How many times was a vertex buffer bound. */
		UINT32 numIndexBufferBinds; /**< How many times was an index buffer bound. */
		UINT32 numGpuParamBufferBinds; /**< How many times was an GPU parameter buffer bound. */
		UINT32 numGpuProgramBinds; /**< How many times was a GPU program bound. */

		UINT32 numResourceWrites; /**< How many times were GPU resources written to. */
		UINT32 numResourceReads; /**< How many times were GPU resources read from. */

		UINT32 numObjectsCreated; /**< How many GPU objects were created. */
		UINT32 numObjectsDestroyed; /**< How many GPU objects were destroyed. */
	};

	/**
	 * @brief	Profiler report containing information about GPU sampling data 
	 *			from a single frame.
	 */
	struct GPUProfilerReport
	{
		GPUProfileSample frameSample; /**< Sample containing data for entire frame. */
		Vector<GPUProfileSample> samples;
	};

	/**
	 * @brief	Profiler that measures time and amount of various GPU operations.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT ProfilerGPU : public Module<ProfilerGPU>
	{
	private:
		struct ActiveSample
		{
			ProfilerString sampleName;
			RenderStats startStats;
			RenderStats endStats;
			TimerQueryPtr activeTimeQuery;
			OcclusionQueryPtr activeOcclusionQuery;
		};

		struct ActiveFrame
		{
			ActiveSample frameSample;
			Vector<ActiveSample> samples;
		};

	public:
		ProfilerGPU();

		/**
		 * @brief	Signals a start of a new frame. Every frame will generate a separate profiling report.
		 *			This call must be followed by "endFrame", and any sampling operations must happen between
		 *			beginFrame and endFrame.
		 */
		void beginFrame();

		/**
		 * @brief	Signals an end of the currently sampled frame. Results of the sampling will be available
		 *			once getNumAvailableReports increments. This may take a while as the sampling is scheduled on
		 *			the core thread and on the GPU.
		 */
		void endFrame();

		/**
		 * @brief	Begins sample measurement. Must be followed by endSample.
		 *
		 * @param	name	Unique name for the sample you can later use to find the sampling data.
		 *
		 * @note	Must be called between beginFrame/endFrame calls.
		 */
		void beginSample(const ProfilerString& name);

		/**
		 * @brief	Ends sample measurement.
		 *
		 * @param	name	Unique name for the sample.
		 *
		 * @note	Unique name is primarily needed to more easily identify mismatched
		 * 			begin/end sample pairs. Otherwise the name in beginSample would be enough.
		 *			Must be called between beginFrame/endFrame calls.
		 */
		void endSample(const ProfilerString& name);

		/**
		 * @brief	Returns number of profiling reports that are ready but haven't been
		 *			retrieved yet. 
		 *
		 * @note	There is an internal limit of maximum number of available reports, where oldest ones will
		 *			get deleted so make sure to call this often if you don't want to miss some.
		 */
		UINT32 getNumAvailableReports();

		/**
		 * @brief	Gets the oldest report available and removes it from the internal list.
		 *			Throws an exception if no reports are available.
		 */
		GPUProfilerReport getNextReport();

		/**
		 * @brief	To be called once per frame from the Core thread.
		 *
		 * @note	Internal method.
		 */
		void _update();

	private:
		/**
		 * @brief	Assigns start values for the provided sample.
		 */
		void beginSampleInternal(ActiveSample& sample);

		/**
		 * @brief	Assigns end values for the provided sample.
		 */
		void endSampleInternal(ActiveSample& sample);

		/**
		 * @brief	Creates a new timer query or returns an existing free query.
		 */
		TimerQueryPtr getTimerQuery() const;

		/**
		 * @brief	Creates a new occlusion query or returns an existing free query.
		 */
		OcclusionQueryPtr getOcclusionQuery() const;

		/**
		 * @brief	Interprets the active frame results and generates a profiler report for
		 *			the frame. Provided frame queries must have finished before calling this.
		 */
		GPUProfilerReport resolveFrame(ActiveFrame& frame);

		/**
		 * @brief	Resolves an active sample and converts it to report sample.
		 */
		void resolveSample(const ActiveSample& sample, GPUProfileSample& reportSample);

	private:
		ActiveFrame mActiveFrame;
		bool mIsFrameActive;
		UINT32 mNumActiveSamples;

		Queue<ActiveFrame> mUnresolvedFrames;
		Queue<GPUProfilerReport> mReadyReports;

		mutable Stack<TimerQueryPtr> mFreeTimerQueries;
		mutable Stack<OcclusionQueryPtr> mFreeOcclusionQueries;
	};
}